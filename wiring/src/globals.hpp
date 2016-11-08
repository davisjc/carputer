
#ifndef WIRING_GLOBALS_H
#define WIRING_GLOBALS_H


/* Pins. */
#define PIN_LED 4
#define PIN_ROTARY_A 15
#define PIN_ROTARY_B 17
#define PIN_SPI_MOSI 10
#define PIN_SPI_CLK 11
#define PIN_SPI_CE 8
#define SPI_CHANNEL 0 /* 0 or 1 */
#define SPI_SPEED_HZ 500000 /* 500000 to 32000000 */

/* Timing. */
#define INT_STARTUP_IGNORE_MS 1000 /* ignore erroneous interrupts at startup */
#define DEBOUNCE_BUTTON_MS 10
#define DEBOUNCE_ROTARY_MS 5

/* Screen harness. */
#define SCREEN_SESSION "carputer_term"
#define SCREEN_USER "eddie"

#define LED_ILLUM_MAX 128


#endif /* WIRING_GLOBALS_H */

