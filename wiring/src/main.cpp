
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


typedef enum UIState {
    CURRENT_PLAYLIST = 0,
    BROWSE,
} UIState;

int
main(void)
{
    gpio::setup();

    /* Fire up the screen session for UI. */
    UIState ui_state = CURRENT_PLAYLIST;
    if (screenharness::is_screen_up(millis())) {
        bool old_screen_killed = screenharness::kill_screen();
        if (old_screen_killed)
            screenharness::spawn_screen(millis());
    } else {
            screenharness::spawn_screen(millis());
    }

    /* Maintain processing queue for input events. */
    std::queue<input::InputEvent> input_events;

    for (;;) {
        uint32_t start_ms = millis();
        int32_t illum_delta = 0;
        UIState ui_state_next = ui_state;

        /* Read all input events since last tick. */
        gpio::read_input_events(input_events);
        while (!input_events.empty()) {
            input::InputEvent event = input_events.front();
            input_events.pop();

            switch (event.id) {
                case input::ROTARY_SPIN_CLOCKWISE:
                    screenharness::enqueue_command(screenharness::SELECT_DOWN);
                    //illum_delta++;
                    break;
                case input::ROTARY_SPIN_CCLOCKWISE:
                    //illum_delta--;
                    screenharness::enqueue_command(screenharness::SELECT_UP);
                    break;
                case input::ROTARY_PRESS:
                    if (!event.mode_shift.blue) {
                        if (ui_state_next == CURRENT_PLAYLIST) {
                            ui_state_next = BROWSE;
                            screenharness::enqueue_command(
                                    screenharness::GOTO_BROWSE);
                        } else if (ui_state_next == BROWSE) {
                            ui_state_next = CURRENT_PLAYLIST;
                            screenharness::enqueue_command(
                                    screenharness::GOTO_CURRENT_PLAYLIST);
                        }
                    }
                    break;
                default:
                    break;
            }
        }

        /* Update LED illumination value. */
        gpio::shift_illum(illum_delta);

        /* Send commands to screen. */
        bool flush_succeeded = screenharness::flush_commands();
        if (flush_succeeded)
            ui_state = ui_state_next;

        /* Restart screen if necessary. */
        if (start_ms - screenharness::get_last_heartbeat_ms() >
                SCREEN_CHECK_INTERVAL_MS) {
            bool is_screen_up = screenharness::is_screen_up(start_ms);
            bool respawn_allowed = (screenharness::get_respawn_retry_count() <
                                    SCREEN_RESPAWN_RETRY_COUNT);
            if (!is_screen_up && respawn_allowed) {
                logger::log("screen session seems to have died; respawning...");
                screenharness::spawn_screen(start_ms);
                ui_state = CURRENT_PLAYLIST;
            }
        }

        /* Cap tick rate. */
        uint32_t cur_time_ms = millis();
        uint32_t interval_ms = cur_time_ms - start_ms;
        if (interval_ms < MIN_MS_TICK) {
            delay(MIN_MS_TICK - interval_ms);
        }
    }

    return 0;
}

