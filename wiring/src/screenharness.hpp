
#ifndef WIRING_SCREENHARNESS_H
#define WIRING_SCREENHARNESS_H


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
    is_screen_up(void);

    /**
     * Spawns a new screen session as a child process.
     *
     * @return true on success; false on failure.
     */
    bool
    spawn_screen(void);

    /**
     * Queue up a command to later be sent with flush_commands().
     */
    void
    enqueue_command(ScreenCommand cmd);

    /**
     * Sends any commands previously set with enqueue_command().
     *
     * NOTE: This also clears the command queue.
     */
    void
    flush_commands(void);
}

#endif /* WIRING_SCREENHARNESS_H */

