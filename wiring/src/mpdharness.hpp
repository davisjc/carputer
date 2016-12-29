
#pragma once

#include <stdint.h>


namespace mpdharness {

    typedef enum MPDCommand {
        NOOP = 0,
        PLAY_PAUSE,
        STOP,
        CLEAR,
        PREV,
        NEXT,
        SEEK_BACK,
        SEEK_BACK_FAST,
        SEEK_FWD,
        SEEK_FWD_FAST,
    } MPDCommand;

    /**
     * Queue up a command to later be sent with flush_commands().
     */
    void
    enqueue_command(MPDCommand cmd);

    /**
     * Sends any commands previously set with enqueue_command().
     *
     * NOTE: This also clears the command queue.
     *
     * @return true if flush succeeds; otherwise false.
     */
    bool
    flush_commands(void);
}

