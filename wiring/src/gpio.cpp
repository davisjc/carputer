
#include "gpio.hpp"

#include <math.h>
#include <pthread.h>
#include <queue>
#include <sstream>
#include <stddef.h>
#include <stdint.h>
#include <string>

#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <softPwm.h>

#include "globals.hpp"
#include "input.hpp"
#include "logger.hpp"


struct ButtonHistory {
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

class InputState {
    std::queue<input::InputEvent> event_queue;
    pthread_mutex_t event_queue_mutex;
public:
    RotaryInfo rotary;
    ButtonInfo button_rotary;
    ButtonInfo button_white_left;
    ButtonInfo button_white_right;
    ButtonInfo button_green;
    ButtonInfo button_red;
    ButtonInfo button_yellow;
    ButtonInfo button_blue;

    InputState()
        : rotary(PIN_ROTARY_A, PIN_ROTARY_B)
        , button_rotary(PIN_BUTTON_ROTARY)
        , button_white_left(PIN_BUTTON_WHITE_LEFT)
        , button_white_right(PIN_BUTTON_WHITE_RIGHT)
        , button_green(PIN_BUTTON_GREEN)
        , button_red(PIN_BUTTON_RED)
        , button_yellow(PIN_BUTTON_YELLOW)
        , button_blue(PIN_BUTTON_BLUE)
    {
        pthread_mutex_init(&event_queue_mutex, NULL);
    }

    ~InputState()
    {
        pthread_mutex_destroy(&event_queue_mutex);
    }

    void
    enqueue_event(input::InputEvent event)
    {
        pthread_mutex_lock(&event_queue_mutex);
        event_queue.push(event);
        pthread_mutex_unlock(&event_queue_mutex);
    }

    void
    consume_events(std::queue<input::InputEvent> &input_events)
    {
        pthread_mutex_lock(&event_queue_mutex);
        while (!event_queue.empty()) {
            input_events.push(event_queue.front());
            event_queue.pop();
        }
        pthread_mutex_unlock(&event_queue_mutex);
    }
};

/*
 * Define a lookup table for button LED brightness ramp of approximately:
 *
 *     y = 2^x - 1 for y = (0, 128); x = (0, ~7.011)
 *
 * Voltage ranges:
 *
 *     MOSFET gate voltage range is about 1.55 V to 2.25 V.
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

static void
set_illum(int level);

static InputState input_state;

static void
register_int_callback(int pin,
                      int int_type, /* INT_EDGE_* as defined by wiringPi */
                      void (*callback)(void));

static void
button_int_callback(ButtonInfo &button_info);

static void
rotary_int_callback(RotaryInfo &rotary_info);

static void
button_int_callback(ButtonInfo &button_info)
{
    int pin_value = digitalRead(button_info.pin);
    if (pin_value == HIGH) {
        logger::log("bouncy.."); // TODO remove
        return; // bouncy.. ignore
    }

    uint32_t cur_ms = millis();

    pthread_mutex_lock(&button_info.mutex);

    uint32_t last_ms = button_info.hist.last_press_ms;
    bool warmup_complete = (cur_ms > INT_STARTUP_IGNORE_MS);
    bool debounce_ellapsed = (cur_ms - DEBOUNCE_BUTTON_MS >= last_ms);
    if (warmup_complete && debounce_ellapsed) {
        logger::log("press!"); // TODO remove
        button_info.hist.last_press_ms = cur_ms;
        //button_info.hist.press_count++; // FIXME
    }

    pthread_mutex_unlock(&button_info.mutex);
}

static void
button_rotary_int_callback(void)
{
    button_int_callback(input_state.button_rotary);
}

static void
button_white_left_int_callback(void)
{
    button_int_callback(input_state.button_white_left);
}

static void
button_white_right_int_callback(void)
{
    button_int_callback(input_state.button_white_right);
}

static void
button_green_int_callback(void)
{
    button_int_callback(input_state.button_green);
}

static void
button_red_int_callback(void)
{
    button_int_callback(input_state.button_red);
}

static void
button_yellow_int_callback(void)
{
    button_int_callback(input_state.button_yellow);
}

static void
button_blue_int_callback(void)
{
    button_int_callback(input_state.button_blue);
}

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
            input::InputEvent event(input::ROTARY_SPIN_CLOCKWISE);
            input_state.enqueue_event(event);
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
            input::InputEvent event(input::ROTARY_SPIN_COUNTERCLOCKWISE);
            input_state.enqueue_event(event);
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
    rotary_int_callback(input_state.rotary);
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
    pinMode(PIN_BUTTON_ROTARY, INPUT);
    pinMode(PIN_BUTTON_WHITE_LEFT, INPUT);
    pinMode(PIN_BUTTON_WHITE_RIGHT, INPUT);
    pinMode(PIN_BUTTON_GREEN, INPUT);
    pinMode(PIN_BUTTON_RED, INPUT);
    pinMode(PIN_BUTTON_YELLOW, INPUT);
    pinMode(PIN_BUTTON_BLUE, INPUT);

    /* Register callbacks. */
    register_int_callback(PIN_ROTARY_A, INT_EDGE_BOTH, rotary_int_callback);
    register_int_callback(PIN_ROTARY_B, INT_EDGE_BOTH, rotary_int_callback);
    register_int_callback(PIN_BUTTON_ROTARY, INT_EDGE_FALLING,
                          button_rotary_int_callback);
    register_int_callback(PIN_BUTTON_WHITE_LEFT, INT_EDGE_FALLING,
                          button_white_left_int_callback);
    register_int_callback(PIN_BUTTON_WHITE_RIGHT, INT_EDGE_FALLING,
                          button_white_right_int_callback);
    register_int_callback(PIN_BUTTON_GREEN, INT_EDGE_FALLING,
                          button_green_int_callback);
    register_int_callback(PIN_BUTTON_RED, INT_EDGE_FALLING,
                          button_red_int_callback);
    register_int_callback(PIN_BUTTON_YELLOW, INT_EDGE_FALLING,
                          button_yellow_int_callback);
    register_int_callback(PIN_BUTTON_BLUE, INT_EDGE_FALLING,
                          button_blue_int_callback);

    /* Set internal pull-up/pull-down registers. */
    pullUpDnControl(PIN_ROTARY_A, PUD_UP);
    pullUpDnControl(PIN_ROTARY_B, PUD_UP);
    pullUpDnControl(PIN_BUTTON_ROTARY, PUD_UP);
    pullUpDnControl(PIN_BUTTON_WHITE_LEFT, PUD_UP);
    pullUpDnControl(PIN_BUTTON_WHITE_RIGHT, PUD_UP);
    pullUpDnControl(PIN_BUTTON_GREEN, PUD_UP);
    pullUpDnControl(PIN_BUTTON_RED, PUD_UP);
    pullUpDnControl(PIN_BUTTON_YELLOW, PUD_UP);
    pullUpDnControl(PIN_BUTTON_BLUE, PUD_UP);

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

void
gpio::read_input_events(std::queue<input::InputEvent> &input_events)
{
    input_state.consume_events(input_events);
}

