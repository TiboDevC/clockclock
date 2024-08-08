#include <Arduino.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "animation.h"
#include "shift_register.h"

/* motor_controller.c */

#define NUM_MOTOR_WIRES      4
#define NUM_MOTORS           (48)
#define NUM_BITS_MOTOR_STEPS (NUM_MOTORS * NUM_MOTOR_WIRES)
#define NUM_STEPS_PER_ROT    (4096)
#define MAX_MOTOR_STEP       (32767)
#define MAX_MOTOR_ANGLE      ((MAX_MOTOR_STEP / NUM_STEPS_PER_ROT) * 360)

#define ANGLE_TO_STEPS(target_angle) (((uint16_t) target_angle * NUM_STEPS_PER_ROT) / (uint16_t) 360)

enum motor_step_t { MOTOR_STEP_0, MOTOR_STEP_1, MOTOR_STEP_2, MOTOR_STEP_3, MOTOR_STEP_OFF, MOTOR_STEP_MAX };

static const int _motor_steps[MOTOR_STEP_MAX] = {
    0b1000,
    0b0100,
    0b0010,
    0b0001,
    0b0000,
};

static struct {
	const struct anim_set_t *anim;
	int current_seq;
} _ctx = {0};

struct motor_status_s {
	uint8_t step_pos  : 2;
	uint8_t anim_type : 1;
};

static struct {
	uint16_t current_step;
	int16_t num_step_remaining;
	uint16_t delay_step_us;
	struct motor_status_s motor_status;
} _motors[NUM_MOTORS] = {0};

static uint8_t _steps[(NUM_MOTOR_WIRES * NUM_MOTORS) / sizeof(uint8_t) + 1] = {0};

static int16_t
    _process_num_steps(uint16_t cur_step, int16_t target_angle, enum anim_angle_type_t anim_angle_type)
{
	int16_t num_step_remaining = 0;
	if (ANGLE_ABSOLUTE == anim_angle_type) {
		/* Correct the angle if necessary */
		target_angle %= 360;
		if (target_angle < 0) {
			target_angle *= -1;
		}

		const uint16_t target_step = ANGLE_TO_STEPS(target_angle);
		/* Always move in the same direction */
		if (target_step > cur_step) {
			num_step_remaining = target_step - cur_step;
		} else {
			num_step_remaining = target_step + ANGLE_TO_STEPS(360) - cur_step;
		}
	} else if (ANGLE_RELATIVE == anim_angle_type) {
		/* Make sure we do not exceed the  max angle */
		if (target_angle > MAX_MOTOR_ANGLE) {
			target_angle = MAX_MOTOR_ANGLE;
		} else if (-MAX_MOTOR_ANGLE > target_angle) {
			target_angle = -MAX_MOTOR_ANGLE;
		}
		num_step_remaining += ANGLE_TO_STEPS(target_angle);
	}
	return num_step_remaining;
}

static int _force_update_motor_anim(const struct anim_pos_t *anim_pos,
                                    uint8_t num_motors,
                                    const struct param_t *param,
                                    int motor_offset)
{
	/* Basic config check */
	if (NULL == anim_pos || NULL == param) {
		return -1;
	}
	for (int motor_idx = 0; motor_idx < num_motors; motor_idx++) {
		if (anim_pos[motor_idx].motor_id + motor_offset >= NUM_MOTORS) {
			/* This motor does not exit, error in the config */
			return -1;
		}
	}

	for (int motor_idx = 0; motor_idx < num_motors; motor_idx++) {
		const uint8_t motor_id = anim_pos[motor_idx].motor_id + motor_offset;
		_motors[motor_id].motor_status.anim_type = param->anim_type;
		_motors[motor_id].num_step_remaining = _process_num_steps(_motors[motor_id].current_step,
		                                                          anim_pos[motor_idx].position_degree,
		                                                          param->anim_angle_type);
		/* TODO process delay_step_us */
	}

	return 0;
}

static int _update_motor_anim(const struct anim_pos_t *anim_pos,
                              uint8_t num_motors,
                              const struct param_t *param,
                              int motor_offset)
{
	/* Basic config check */
	if (NULL == anim_pos || NULL == param) {
		return -1;
	}
	for (int motor_idx = 0; motor_idx < num_motors; motor_idx++) {
		if (anim_pos[motor_idx].motor_id + motor_offset >= NUM_MOTORS) {
			/* This motor does not exit, error in the config */
			return -1;
		}
	}

	/* Check if animation is in blocking mode and not finished */
	for (int motor_idx = 0; motor_idx < NUM_MOTORS; motor_idx++) {
		if (ANIM_SYNC == _motors[motor_idx].motor_status.anim_type &&
		    0 != _motors[motor_idx].num_step_remaining) {
			/* At least one animation is not done */
			return -1;
		}
	}

	/* All animations are done, apply the new one */
	_force_update_motor_anim(anim_pos, num_motors, param, motor_offset);

	return 0;
}

bool is_anim_done(void)
{
	for (int motor_id = 0; motor_id < NUM_MOTORS; motor_id++) {
		if (_motors[motor_id].num_step_remaining != 0) {
			return false;
		}
	}
	return true;
}

bool is_anim_ready(void)
{
	for (int motor_id = 0; motor_id < NUM_MOTORS; motor_id++) {
		if (_motors[motor_id].num_step_remaining != 0 &&
		    ANIM_SYNC == _motors[motor_id].motor_status.anim_type) {
			return false;
		}
	}
	return true;
}

void print_motor_state(void)
{
	for (int motor_id = 0; motor_id < NUM_MOTORS; motor_id++) {
		if (_motors[motor_id].num_step_remaining != 0) {
			printf("%d: %d/%d (%d), type %d\n",
			       motor_id,
			       _motors[motor_id].current_step,
			       _motors[motor_id].num_step_remaining,
			       _motors[motor_id].motor_status.step_pos,
			       _motors[motor_id].motor_status.anim_type);
		}
	}
}

static void _set_motor_bits(int motor_id, int step_id)
{
	const int bit_shift = (motor_id % 2) * 4;
	const int bit_mask = (motor_id % 2) == 0 ? 0b00001111 : 0b11110000;
	const int motor_step = (_motor_steps[step_id] << bit_shift) & bit_mask;
	_steps[motor_id / 2] &= ~bit_mask;
	_steps[motor_id / 2] |= motor_step;
}

static void _update_current_anim(void)
{
	const int seq = _ctx.current_seq;
	const struct anim_sequence_t *sequence = &_ctx.anim->sequence[seq];

	if (NULL == sequence) {
		return;
	}

	for (int motor_id = 0; motor_id < sequence->num_motors; motor_id++) {
		_update_motor_anim(sequence->position, sequence->num_motors, &sequence->param, 0);
	}
}

void anim_set(const struct anim_set_t *anim_set)
{
	/* Switch to new anim, reset the ctx */
	_ctx.anim = anim_set;
	_ctx.current_seq = 0;

	if (NULL == anim_set || 0 == anim_set[0].num_sequences) {
		return;
	}

	_update_current_anim();
}

void seq_set(const struct anim_sequence_t *seq, int motor_offset)
{
	/* Remove current animation */
	_ctx.anim = NULL;
	_ctx.current_seq = 0;

	if (NULL == seq) {
		return;
	}

	for (int motor_id = 0; motor_id < seq->num_motors; motor_id++) {
		_force_update_motor_anim(seq->position, seq->num_motors, &seq->param, motor_offset);
	}
}

#define MOTOR_DELAY_US 5000
int process_anim(void)
{
	static unsigned long last_time_us = 0;
	unsigned long time_us = micros();

	if (time_us > last_time_us && time_us - last_time_us < MOTOR_DELAY_US) {
		return -1;
	}
	last_time_us = time_us;

	/* Process current animation */
	for (int motor_id = 0; motor_id < NUM_MOTORS; motor_id++) {
		const int16_t num_step_remaining = _motors[motor_id].num_step_remaining;
		if (num_step_remaining > 0) {
			_motors[motor_id].num_step_remaining--;
			_motors[motor_id].current_step--;

			if (_motors[motor_id].motor_status.step_pos == MOTOR_STEP_0) {
				_motors[motor_id].motor_status.step_pos = MOTOR_STEP_3;
			} else {
				_motors[motor_id].motor_status.step_pos--;
			}

			_set_motor_bits(motor_id, _motors[motor_id].motor_status.step_pos);
		} else if (num_step_remaining < 0) {
			_motors[motor_id].num_step_remaining++;
			_motors[motor_id].current_step++;

			_motors[motor_id].motor_status.step_pos++;
			_motors[motor_id].motor_status.step_pos %= MOTOR_STEP_OFF;

			_set_motor_bits(motor_id, _motors[motor_id].motor_status.step_pos);
		} else {
			printf("Motor off\n");
			_set_motor_bits(motor_id, MOTOR_STEP_OFF);
		}
	}

	ctrl_motors(_steps, NUM_BITS_MOTOR_STEPS);

	if (is_anim_done()) {
		if (_ctx.current_seq < _ctx.anim->num_sequences) {
			_ctx.current_seq++;
			_update_current_anim();
		} else {
			/* Sequence is finished */
		}
	}
	return 0;
}

static const struct anim_pos_t _pos_test[] = {
    {.position_degree = 180, .motor_id = 0},
    {.position_degree = 90, .motor_id = 1},
};

static const struct anim_sequence_t _seq_test[] = {
    {.num_motors = sizeof(_pos_test) / sizeof(_pos_test[0]),
     .param =
         {
	     .anim_type = ANIM_SYNC,
	     .anim_angle_type = ANGLE_ABSOLUTE,
	     .time_ms = 20,
	 },
     .position = _pos_test},
};

void test_anim(void)
{
	seq_set(_seq_test, 0);
}
