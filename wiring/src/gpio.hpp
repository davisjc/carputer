
#ifndef WIRING_GPIO_H
#define WIRING_GPIO_H

#include "globals.hpp"


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
     * Reports the rotary spin since last read.
     *
     * Positve values indicate clockwise rotations while negative indicate
     * counter-clockwise.
     *
     * NOTE: This call consumes the spin value. Immediate successive calls will
     *       report 0.
     */
    int
    read_rotary_spin_value(void);
}

#endif /* WIRING_GPIO_H */

