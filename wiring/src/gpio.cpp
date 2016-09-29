
#include "gpio.hpp"

#include <string>
#include <sstream>
#include <stdint.h>
#include <math.h>
#include <pthread.h>
#include <wiringPi.h>
#include <softPwm.h>

#include "types.hpp"
#include "logger.hpp"


/* Lookup table for approximate values of y = 2^x */
static int illum_steps[] = {
    0,
    1,
    2,
    3,
    4,
    5,
    7,
    9,
    12,
    16,
    21,
    28,
    37,
    48,
    63,
    84,
    111,
    147,
    193,
    LED_ILLUM_MAX,
};
static const int illum_step_count = (sizeof(illum_steps) /
                                        sizeof(illum_steps[0]));
static int illum_i = illum_step_count / 2; // TODO serialize last setting

static InputState inputs;

static void
register_int_callback(int pin,
                      int int_type, /* INT_EDGE_* as defined by wiringPi */
                      void (*callback)(void));

//static void
//button_int_callback(ButtonInfo& button_info);

static void
rotary_int_callback(RotaryInfo& rotary_info);

//static void
//button_int_callback(ButtonInfo& button_info)
//{
//    uint32_t cur_ms = millis();
//
//    pthread_mutex_lock(&button_info.mutex);
//
//    uint32_t last_ms = button_info.hist.last_press_ms;
//    bool warmup_complete = (cur_ms > INT_STARTUP_IGNORE_MS);
//    bool debounce_ellapsed = (cur_ms - DEBOUNCE_BUTTON_MS >= last_ms);
//    if (warmup_complete && debounce_ellapsed) {
//        button_info.hist.last_press_ms = cur_ms;
//        button_info.hist.press_count++;
//    }
//
//    pthread_mutex_unlock(&button_info.mutex);
//}

//static void
//button_1_int_callback(void)
//{
//    button_int_callback(inputs.button_1);
//}
//
//static void
//button_2_int_callback(void)
//{
//    button_int_callback(inputs.button_2);
//}

static void
rotary_int_callback(RotaryInfo& rotary_info)
{
    int pin_a_value = digitalRead(rotary_info.pin_a);
    int pin_b_value = digitalRead(rotary_info.pin_b);
    int cur_bits = (pin_a_value << 1) | pin_b_value;
    uint32_t cur_ms = millis();

    pthread_mutex_lock(&rotary_info.mutex);

    uint32_t last_ms = rotary_info.hist.last_spin_ms;
    bool warmup_complete = (cur_ms > INT_STARTUP_IGNORE_MS);
    bool debounce_ellapsed = (cur_ms - DEBOUNCE_ROTARY_MS >= last_ms);
    if (warmup_complete && debounce_ellapsed) {
        /* Bits by position (0 is LSB):
         *   3 - last value of pin_a
         *   2 - last value of pin_b
         *   1 - current value of pin_a
         *   0 - current value of pin_b */
        int bits_edge = (rotary_info.hist.last_bits << 2) | cur_bits;

        /* Detect signal using Gray code:
         * https://en.wikipedia.org/wiki/Gray_code
         *
         * All edge transitions (where MSB is pin A and LSB is pin B):
         *
         *   clockwise                  counter-clockwise
         *   =========                  =================
         *   10 -> 11                   00 -> 01 <--- good feel!
         *   11 -> 01                   01 -> 11
         *   01 -> 00 <--- good feel!   11 -> 10
         *   00 -> 10                   10 -> 00
         *
         * All 4 of these get fired every dedent tick of the spin. So if you
         * want one signal per detent tick, just use one of the 4. Each one
         * feels a bit different as each one corresponds to a certain point in
         * the phase of rotation. The best ones can be picked through
         * experimentation with the particular rotary encoder.
         */
        if (bits_edge == 0b0100) {
            rotary_info.hist.spin_value++;
#ifdef DEBUG
            {
                std::ostringstream msg;
                for (int b = 3; b >= 0; b--)
                    msg << ((bits_edge & (1 << b)) ? "1" : "0");
                msg << " " << millis() << " clockwise!";
                logger::log(msg.str());
            }
#endif
        } else if (bits_edge == 0b0001) {
            rotary_info.hist.spin_value--;
#ifdef DEBUG
            {
                std::ostringstream msg;
                for (int b = 3; b >= 0; b--)
                    msg << ((bits_edge & (1 << b)) ? "1" : "0");
                msg << " " << millis() << " counter-clockwise!";
                logger::log(msg.str());
            }
#endif
        } else {
            /* 0 or 2 bits changed at once */
        }

        rotary_info.hist.last_spin_ms = cur_ms;
    }

    rotary_info.hist.last_bits = cur_bits;

    pthread_mutex_unlock(&rotary_info.mutex);
}

static void
rotary_int_callback()
{
    rotary_int_callback(inputs.rotary);
}

static void
register_int_callback(int pin, int int_type, void (*callback)(void))
{
    pinMode(pin, INPUT);
    if (wiringPiISR(pin, int_type, callback)) {
        logger::error("wiringPiISR() error");
        exit(1);
    }
}

void
gpio::setup(void)
{
#ifdef DEBUG
    logger::log("Setting up GPIO...");
#endif
    wiringPiSetupGpio();

    if (softPwmCreate(PIN_LED, 0, LED_ILLUM_MAX)) {
        logger::error("softPwmCreate() error");
        exit(1);
    }

    /* Initialize input state holder. */
    // TODO create classes for input state
    inputs.rotary.pin_a = PIN_ROTARY_A;
    inputs.rotary.pin_b = PIN_ROTARY_B;

    /* Register callbacks. */
    register_int_callback(PIN_ROTARY_A, INT_EDGE_BOTH, rotary_int_callback);
    register_int_callback(PIN_ROTARY_B, INT_EDGE_BOTH, rotary_int_callback);

    /* Set internal pull-up/pull-down registers. */
    pullUpDnControl(PIN_ROTARY_A, PUD_UP);
    pullUpDnControl(PIN_ROTARY_B, PUD_UP);

    /* Start PWM of illumination LEDs. */
    softPwmWrite(PIN_LED, illum_steps[illum_i]);

#ifdef DEBUG
    logger::log("GPIO initialized.");
#endif
}

void
gpio::shift_illum(int levels)
{
    illum_i += levels;
    if (illum_i >= illum_step_count) {
        illum_i = illum_step_count - 1;
    } else if (illum_i < 0) {
        illum_i = 0;
    }
    softPwmWrite(PIN_LED, illum_steps[illum_i]);
}

int
gpio::read_rotary_spin_value(void)
{
    int spin_value;
    pthread_mutex_lock(&inputs.rotary.mutex);
    spin_value = inputs.rotary.hist.spin_value;
    inputs.rotary.hist.spin_value = 0;
    pthread_mutex_unlock(&inputs.rotary.mutex);
    return spin_value;
}

