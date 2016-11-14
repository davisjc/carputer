
#pragma once

namespace input {

    /* Define identifiers for various input events. */
    typedef enum InputId {
        ROTARY_PRESS = 0,
        ROTARY_SPIN_CCLOCKWISE,
        ROTARY_SPIN_CLOCKWISE,
        WHITE_LEFT,
        WHITE_RIGHT,
        GREEN,
        RED,
        YELLOW,
        BLUE
    } ButtonIds;

    /* Describes a current mode shift state. */
    struct ModeShift {
        bool rotary = false;
        bool yellow = false;
        bool blue = false;
    };

    /* Define an input event. */
    struct InputEvent {
        InputId id;
        ModeShift mode_shift;

        InputEvent(InputId input_id)
            : id(input_id)
        {}

        InputEvent(InputId input_id, ModeShift mode_shift_value)
            : id(input_id)
            , mode_shift(mode_shift_value)
        {}
    };
}

