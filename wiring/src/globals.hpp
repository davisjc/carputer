
#pragma once

/* Pins. */
#define PIN_ROTARY_A 17
#define PIN_ROTARY_B 27
#define PIN_BUTTON_ROTARY 22
#define PIN_BUTTON_WHITE_LEFT 5
#define PIN_BUTTON_WHITE_RIGHT 6
#define PIN_BUTTON_GREEN 13
#define PIN_BUTTON_YELLOW 23
//#define PIN_BUTTON_BLUE 24 /* TODO revert once pin fixed */
#define PIN_BUTTON_BLUE 7
#define PIN_BUTTON_RED 12
#define PIN_IGNITION_SENSE 16
#define PIN_SPI_MOSI 10
#define PIN_SPI_CLK 11
#define PIN_SPI_CE 8
#define SPI_CHANNEL 0 /* 0 or 1 */
#define SPI_SPEED_HZ 500000 /* 500000 to 32000000 */

/* Timing. */
#define INT_STARTUP_IGNORE_MS 1000 /* ignore erroneous interrupts at startup */
#define DEBOUNCE_BUTTON_MS 100
#define DEBOUNCE_ROTARY_MS 5

/* Screen harness. */
#define SCREEN_SESSION "carputer_term"
#define SCREEN_USER "eddie"
#define SCREEN_RESPAWN_RETRY_COUNT 10
#define SCREEN_CHECK_INTERVAL_MS 5 * 1000

/* MPD harness */
#define MPD_SEEK_PCT "1%"
#define MPD_SEEK_FAST_PCT "5%"
#define MPD_USER "eddie"

#define LED_ILLUM_MAX 128

