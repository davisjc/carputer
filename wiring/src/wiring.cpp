
#include <iostream>
#include <stdint.h>
#include <wiringPi.h>

#include "gpio.hpp"
#include "types.hpp"

#define MAX_TICKS_S 30 /* cap ticks per second */
#define MIN_MS_TICK 1000.0 / MAX_TICKS_S /* min milliseconds every tick */


int
main(void)
{
    gpio::setup();

    for (;;) {
        uint32_t start_ms = millis();

        int rotary_spin_value = gpio::read_rotary_spin_value();

        /* Update LED illumination value. */
        gpio::shift_illum(rotary_spin_value);

        /* Cap tick rate. */
        uint32_t cur_time_ms = millis();
        uint32_t interval_ms = cur_time_ms - start_ms;
        if (interval_ms < MIN_MS_TICK) {
            delay(MIN_MS_TICK - interval_ms);
        }
    }

    return 0;
}
