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
  uint8_t motor_id;
  enum anim_type_t anim_type;
  enum anim_angle_type_t anim_angle_type;
  int16_t position_degree;
  uint16_t time_ms;
};

struct anim_sequence_t {
  int num_motors;
  struct anim_pos_t anim_pos[MAX_POSITIONS];
};

struct anim_set_t {
  int num_sequences;
  struct anim_sequence_t anim_sequence[MAX_SEQUENCES];
};

int send_anim(const struct anim_pos_t* anim_pos, uint8_t num_anim);
void print_motor_state(void);
int process_anim(void);
bool is_anim_done(void);
bool is_anim_ready(void);

#ifdef __cplusplus
}
#endif

#endif /* ANIMATION_H */
