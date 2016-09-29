
#ifndef WIRING_GPIO_H
#define WIRING_GPIO_H


/* Pins. */
#define PIN_LED 4
#define PIN_ROTARY_A 15
#define PIN_ROTARY_B 17

/* Timing. */
#define INT_STARTUP_IGNORE_MS 1000 /* ignore erroneous interrupts at startup */
#define DEBOUNCE_BUTTON_MS 10
#define DEBOUNCE_ROTARY_MS 4

#define LED_ILLUM_MAX 255

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

