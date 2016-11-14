
#pragma once

#include "globals.hpp"
#include "input.hpp"

#include <pthread.h>
#include <queue>
#include <stddef.h>
#include <stdint.h>


namespace gpio {

    /**
     * Setup the GPIO for use by wiringPi.
     *
     * WARNING: Only call this once.
     */
    void
    setup(void);

    /**
     * Changes the current LED illumination.
     */
    void
    shift_illum(int steps /* shifts brightness by this amount */);

    /**
     * Populates a queue with input events since last call.
     *
     * NOTE: This call consumes the input events. Immediate successive calls
     *       will not find nothing.
     */
    void
    read_input_events(std::queue<input::InputEvent> &input_events);
}

