
#pragma once

#include <stdint.h>


namespace mpdharness {

    /**
     * Toggle play/pause state.
     */
    void
    playpause(void);

    /**
     * Stop playback.
     */
    void
    stop(void);

    /**
     * Clears playback queue.
     */
    void
    clear(void);

    /**
     * Previous track.
     */
    void
    previous(void);

    /**
     * Next track.
     */
    void
    next(void);

    /**
     * Seek in current track.
     */
    void
    seek(int32_t offset_s);
}

