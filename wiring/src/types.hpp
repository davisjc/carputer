
#ifndef WIRING_TYPES_H
#define WIRING_TYPES_H

#include <stdint.h>
#include <pthread.h>


typedef struct ButtonHistory {
    uint32_t press_count;
    uint32_t last_press_ms;
} ButtonHistory;

typedef struct RotaryHistory {
    int32_t spin_value; /* clockwise is pos; counter-clockwise is neg */

    /* 2 bits indicate last pin state:
     *     MSB - pin_a
     *     LSB - pin_b */
    int last_bits;
    uint32_t last_spin_ms;
} RotaryHistory;

typedef struct ButtonInfo {
    int pin; // TODO make const
    ButtonHistory hist;
    pthread_mutex_t mutex;
} ButtonInfo;

typedef struct RotaryInfo {
    int pin_a; // TODO make const
    int pin_b; // TODO make const
    RotaryHistory hist;
    pthread_mutex_t mutex;
} RotaryInfo;

typedef struct InputState {
    ButtonInfo button_1;
    ButtonInfo button_2;
    RotaryInfo rotary;
} InputState;

#endif /* WIRING_TYPES_H */

