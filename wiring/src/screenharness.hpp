
#pragma once

#include <stdint.h>


namespace screenharness {

    typedef enum ScreenCommand {
        NOOP = 0,

        /* Global view commands. */
        SELECT_UP,
        SELECT_DOWN,
        SELECT_LEFT,
        SELECT_RIGHT,
        SELECT_UP_PAGE,
        SELECT_DOWN_PAGE,
        SELECT_TOP,
        SELECT_BOTTOM,
        PLAY_SELECTED,
        FIND_CURRENT_PLAYING,
        CLEAR,

        /* Additional current playlist view commands. */
        MOVE_UP,
        MOVE_DOWN,
        REMOVE,
        GOTO_BROWSE,

        /* Additional browsing view commands. */
        ADD_TO_PLAYING,
        GOTO_CURRENT_PLAYLIST,
    } ScreenCommand;

    /**
     * @return whether or not the screen session is running.
     */
    bool
    is_screen_up(uint32_t cur_ms);

    /**
     * Spawns a new screen session as a child process.
     *
     * @return true on success; false on failure.
     */
    bool
    spawn_screen(uint32_t cur_ms);

    /**
     * Kills the active screen session.
     */
    bool
    kill_screen(void);

    /**
     * Queue up a command to later be sent with flush_commands().
     */
    void
    enqueue_command(ScreenCommand cmd);

    /**
     * Sends any commands previously set with enqueue_command().
     *
     * NOTE: This also clears the command queue.
     *
     * @return true if flush succeeds; otherwise false.
     */
    bool
    flush_commands(void);

    /**
     * @return the time in milliseconds the screen was last seen up.
     */
    uint32_t
    get_last_heartbeat_ms(void);

    /**
     * @return the number of successive respawn attempts have failed.
     */
    uint32_t
    get_respawn_retry_count(void);
}

