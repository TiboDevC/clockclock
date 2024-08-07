#ifndef ANIMATION_H
#define ANIMATION_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define MAX_SEQUENCES 6
#define MAX_POSITIONS 48

enum anim_type_t {
    ANIM_SYNC,
    ANIM_ASYNC,
};

enum anim_angle_type_t {
    ANGLE_ABSOLUTE,
    ANGLE_RELATIVE,
};

struct anim_pos_t {
    int16_t position_degree;
};

struct anim_sequence_t {
    struct {
        uint8_t num_motors : 6;
        enum anim_type_t anim_type : 1;
        enum anim_angle_type_t anim_angle_type : 1;
        uint16_t time_ms : 7;
    };
    struct anim_pos_t position[MAX_POSITIONS];
};

struct anim_set_t {
    int num_sequences;
    struct anim_sequence_t sequence[MAX_SEQUENCES];
};

void print_motor_state(void);
int process_anim(void);
bool is_anim_done(void);
bool is_anim_ready(void);

#ifdef __cplusplus
}
#endif

#endif /* ANIMATION_H */
