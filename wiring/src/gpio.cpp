
#include "gpio.hpp"

#include <string>
#include <sstream>
#include <stdint.h>
#include <math.h>
#include <pthread.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <softPwm.h>

#include "logger.hpp"


struct ButtonHistory {
    uint32_t press_count = 0;
    uint32_t last_press_ms = 0;
};

struct ButtonInfo {
    const int pin;
    ButtonHistory hist;
    pthread_mutex_t mutex;

    ButtonInfo(const int pin_id)
        : pin(pin_id)
    {
        pthread_mutex_init(&mutex, NULL);
    }

    ~ButtonInfo()
    {
        pthread_mutex_destroy(&mutex);
    }
};

struct RotaryHistory {
    int32_t spin_value = 0; /* clockwise is pos; counter-clockwise is neg */

    /* 2 bits indicate last pin state:
     *     MSB - pin_a
     *     LSB - pin_b */
    int last_bits = 0;
    uint32_t last_spin_ms = 0;
};

struct RotaryInfo {
    const int pin_a;
    const int pin_b;
    RotaryHistory hist;
    pthread_mutex_t mutex;

    RotaryInfo(const int pin_a_id, const int pin_b_id)
        : pin_a(pin_a_id)
        , pin_b(pin_b_id)
    {
        pthread_mutex_init(&mutex, NULL);
    }

    ~RotaryInfo()
    {
        pthread_mutex_destroy(&mutex);
    }
};

struct InputState {
    //ButtonInfo button_1;
    //ButtonInfo button_2;
    RotaryInfo rotary;

    InputState()
        : rotary(PIN_ROTARY_A, PIN_ROTARY_B)
    {}
};

/*
 * Define a lookup table for button LED brightness ramp of approximately:
 *
 *     y = 2^x - 1 for y = (0, 128); x = (0, ~7.011)
 *
 * Voltage ranges:
 *
 *     MOSFET voltage gate range is about 1.55 V to 2.25 V.
 *     The desired button LED positive rail range is from 2.1 V - 3.3 V
 *
 * In-series resistors can be used above/below a digital potentiometer with
 * resistance of 9.68 kOhms operating at 3.3 V to obtain the following voltages
 * ranges available through the wiper pin, allowing finer control over the
 * MOSFET gate voltage:
 *
 *     10 kOhms and 15 kOhms    = 1.45 V - 2.35 V
 *     10 kOhms and 16 kOhms    = 1.50 V - 2.38 V
 *     84.2 kOhms and 203 kOhms = 2.25 V - 2.36 V
 *
 * Minimum visible voltage for button LEDs:
 *
 *     red    = 1.55 V
 *     yellow = 1.73 V
 *     green  = 1.97 V
 *     blue   = 2.31 V
 *     white  = 2.39 V
 */
static int illum_steps[] = {
    0,
    1,
    2,
    3,
    4,
    5,
    6,
    7,
    8,
    9,
    10,
    12,
    13,
    15,
    17,
    20,
    22,
    25,
    29,
    33,
    37,
    42,
    48,
    54,
    61,
    69,
    78,
    88,
    99,
    112,
    LED_ILLUM_MAX,
};
static const int illum_step_count = (sizeof(illum_steps) /
                                     sizeof(illum_steps[0]));
static int illum_i = (int)(illum_step_count * 3.0 / 4); // TODO serialize

static InputState inputs;

static void
set_illum(int level);

static void
register_int_callback(int pin,
                      int int_type, /* INT_EDGE_* as defined by wiringPi */
                      void (*callback)(void));

//static void
//button_int_callback(ButtonInfo &button_info);

static void
rotary_int_callback(RotaryInfo &rotary_info);

//static void
//button_int_callback(ButtonInfo &button_info)
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
rotary_int_callback(RotaryInfo &rotary_info)
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
         *   10 -> 11                   00 -> 01 <--- good feel! (drops often..)
         *   11 -> 01                   01 -> 11
         *   01 -> 00 <--- good feel!   11 -> 10
         *   00 -> 10                   10 -> 00 <--- good balance of feel/drop
         *
         * All 4 transitions are incurred during each detent tick. Listening to
         * just one of these should report a single tick has spun. Note that
         * there is a lag between the interrupt and reading both pin values.
         * However, this still catches most of the ticks.
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
        } else if (bits_edge == 0b1000) {
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
    wiringPiSPISetup(0, SPI_SPEED_HZ);

    /* Set pin modes. */
    // TODO buttons

    /* Register callbacks. */
    register_int_callback(PIN_ROTARY_A, INT_EDGE_BOTH, rotary_int_callback);
    register_int_callback(PIN_ROTARY_B, INT_EDGE_BOTH, rotary_int_callback);
    // TODO buttons

    /* Set internal pull-up/pull-down registers. */
    pullUpDnControl(PIN_ROTARY_A, PUD_UP);
    pullUpDnControl(PIN_ROTARY_B, PUD_UP);
    // TODO buttons

    /* Set initial LED brightness. */
    set_illum(illum_i);

#ifdef DEBUG
    logger::log("GPIO initialized.");
#endif
}

static void
set_illum(int level)
{
    uint8_t buf[3] = { 1, 0, 0 };
    buf[1] = illum_steps[level];
#ifdef DEBUG
    {
        std::ostringstream msg;
        msg << "brightness: " << (int)buf[1];
        logger::log(msg.str());
    }
#endif
    wiringPiSPIDataRW(0, buf, sizeof(buf)/sizeof(buf[0]));
}

void
gpio::shift_illum(int levels)
{
    if (levels == 0)
        return;

    illum_i += levels;
    if (illum_i >= illum_step_count) {
        illum_i = illum_step_count - 1;
    } else if (illum_i < 0) {
        illum_i = 0;
    }
    set_illum(illum_i);
}

int
gpio::read_rotary_tick_value(void)
{
    int spin_value;
    pthread_mutex_lock(&inputs.rotary.mutex);
    spin_value = inputs.rotary.hist.spin_value;
    inputs.rotary.hist.spin_value = 0;
    pthread_mutex_unlock(&inputs.rotary.mutex);
    return spin_value;
}

