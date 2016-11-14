
#include <iostream>
#include <queue>
#include <stdint.h>
#include <wiringPi.h>

#include "gpio.hpp"
#include "input.hpp"
#include "logger.hpp"
#include "screenharness.hpp"

#define MAX_TICKS_S 30 /* cap ticks per second */
#define MIN_MS_TICK 1000.0 / MAX_TICKS_S /* min milliseconds every tick */


int
main(void)
{
    gpio::setup();

    if (!screenharness::is_screen_up())
        screenharness::spawn_screen();

    std::queue<input::InputEvent> input_events;

    for (;;) {
        uint32_t start_ms = millis();
        int32_t rotary_spin_value = 0;
        //bool send_screen_commands = false;

        /* Read all input events since last tick. */
        gpio::read_input_events(input_events);
        while (!input_events.empty()) {
            input::InputEvent event = input_events.front();
            input_events.pop();

            switch (event.id) {
                case input::ROTARY_SPIN_CLOCKWISE:
                    rotary_spin_value++;
                    break;
                case input::ROTARY_SPIN_COUNTERCLOCKWISE:
                    rotary_spin_value--;
                    break;
                default:
                    break;
            }
        }

        /* Update LED illumination value. */
        gpio::shift_illum(rotary_spin_value);

        //for (int i = rotary_tick_value; i != 0; ) {
            //send_screen_commands = true;
            //if (i < 0) {
                //screenharness::enqueue_command(screenharness::SELECT_UP);
                //i++;
            //} else {
                //screenharness::enqueue_command(screenharness::SELECT_DOWN);
                //i--;
            //}
        //}

        //if (send_screen_commands)
            //screenharness::flush_commands();

        /* Cap tick rate. */
        uint32_t cur_time_ms = millis();
        uint32_t interval_ms = cur_time_ms - start_ms;
        if (interval_ms < MIN_MS_TICK) {
            delay(MIN_MS_TICK - interval_ms);
        }
    }

    return 0;
}

