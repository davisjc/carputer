
#ifndef WIRING_SCREENHARNESS_H
#define WIRING_SCREENHARNESS_H

#include <string>


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

    bool
    is_screen_up(void);

    bool
    spawn_screen(void);

    void
    enqueue_command(ScreenCommand cmd);

    void
    flush_commands(void);
}

#endif /* WIRING_SCREENHARNESS_H */

