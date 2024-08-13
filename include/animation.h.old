#ifndef ANIMATION_H
#define ANIMATION_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

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
	int16_t position_degree : 10;
	uint8_t motor_id        : 6;
};

struct anim_sequence_t {
	uint8_t num_motors;
	struct param_t {
		enum anim_type_t anim_type             : 1;
		enum anim_angle_type_t anim_angle_type : 1;
		uint8_t time_ms                        : 6;
	} param;
	const struct anim_pos_t *position;
};

struct anim_set_t {
	uint8_t num_sequences;
	const struct anim_sequence_t *sequence;
};

void print_motor_state(void);
int process_anim(void);
bool is_anim_done(void);
bool is_anim_ready(void);
void anim_set(const struct anim_set_t *anim_set);
void test_anim(void);
void seq_set(const struct anim_sequence_t *seq, int motor_offset);

#ifdef __cplusplus
}
#endif

#endif /* ANIMATION_H */
