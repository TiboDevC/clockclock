#include "Arduino.h"

#include "cfg.hpp"
#include "motion.hpp"
#include "motor_motion.h"

#ifdef DEBUG_MOTION
#define DBG_MOTION(...)    Serial.print(__VA_ARGS__)
#define DBG_MOTION_LN(...) Serial.println(__VA_ARGS__)
#else
#define DBG_MOTION(...)
#define DBG_MOTION_LN(...)
#endif

#define ANGLE_TO_STEPS(target_angle) \
	((uint16_t) (((uint32_t) target_angle * NUM_STEPS_PER_ROT) / (uint32_t) 360))
#define STEP_TO_ANGLE(target_step) (((uint32_t) target_step * 360ul) / NUM_STEPS_PER_ROT)

typedef uint16_t pos_t;

enum transition_t : uint8_t {
	TRANS_SHORTER_PATH,
	TRANS_CLOCKWISE,
};

/******************************************************************************************************************/

/**
 * Digits
 * structure: {
 * h0, m0,
 * h1, m1,
 * ...
 * h5, m5
 * }
 */

#define NUM_MOTOR_PER_DIAL 2
#define NUM_DIAL_PER_DIGIT 6
#define NUM_DIGIT          4

typedef uint16_t angle_t;

struct clock_dial_t {
	angle_t angle_absolute[NUM_MOTOR_PER_DIAL];
};

struct clock_digit_t {
	clock_dial_t clocks[NUM_DIAL_PER_DIGIT];
};

struct full_clock_t {
	clock_digit_t digit[NUM_DIGIT];
};

static constexpr struct {
	transition_t transition;
} _ctx = {.transition = TRANS_CLOCKWISE};

static constexpr clock_digit_t digit_0 = {
    .clocks = {{90, 180}, {0, 180}, {0, 90}, {180, 270}, {0, 180}, {270, 0}}};

static constexpr clock_digit_t digit_1 = {
    .clocks = {{225, 225}, {225, 225}, {225, 225}, {0, 180}, {0, 180}, {0, 180}}};

static constexpr clock_digit_t digit_2 = {
    .clocks = {{90, 90}, {90, 180}, {90, 0}, {270, 180}, {90, 270}, {270, 270}}};

static constexpr clock_digit_t digit_3 = {
    .clocks = {{90, 90}, {90, 90}, {90, 90}, {270, 180}, {180, 0}, {270, 0}}};

static constexpr clock_digit_t digit_4 = {
    .clocks = {{180, 180}, {0, 90}, {225, 225}, {180, 180}, {0, 180}, {0, 0}}};

static constexpr clock_digit_t digit_5 = {
    .clocks = {{90, 180}, {0, 90}, {90, 90}, {270, 270}, {180, 270}, {270, 0}}};

static constexpr clock_digit_t digit_6 = {
    .clocks = {{180, 180}, {0, 180}, {0, 90}, {45, 45}, {270, 180}, {0, 270}}};

static constexpr clock_digit_t digit_7 = {
    .clocks = {{90, 90}, {225, 225}, {225, 225}, {270, 180}, {0, 180}, {0, 180}}};

static constexpr clock_digit_t digit_8 = {
    .clocks = {{90, 180}, {90, 180}, {0, 90}, {270, 180}, {270, 180}, {0, 270}}};

static constexpr clock_digit_t digit_9 = {
    .clocks = {{90, 180}, {0, 90}, {225, 225}, {180, 270}, {0, 180}, {0, 0}}};

static constexpr clock_digit_t digit_null = {
    .clocks = {{270, 270}, {270, 270}, {270, 270}, {270, 270}, {270, 270}, {270, 270}}};

static constexpr clock_digit_t digit_I = {
    .clocks = {{270, 90}, {270, 90}, {270, 90}, {270, 90}, {270, 90}, {270, 90}}};

static constexpr clock_digit_t digit_fun = {.clocks = {
						{225, 45},
						{225, 45},
						{225, 45},
						{225, 45},
						{225, 45},
						{225, 45},
					    }};

static constexpr clock_digit_t _digits[10] = {
    digit_0, digit_1, digit_2, digit_3, digit_4, digit_5, digit_6, digit_7, digit_8, digit_9};

static full_clock_t _get_clock_state_from_time(const int h, const int m)
{
	DBG_MOTION("Set time: ");
	DBG_MOTION(h);
	DBG_MOTION(":");
	DBG_MOTION_LN(m);

	const int d0 = h / 10;
	const int d1 = h - d0 * 10;
	const int d2 = m / 10;
	const int d3 = m - d2 * 10;

	const full_clock_t clock_state = {_digits[d0], _digits[d1], _digits[d2], _digits[d3]};
	return clock_state;
}

static void _shortest_path(const int motor_idx, const pos_t target_pos_absolute)
{
	long cur_position = motor_get_position(motor_idx);
	if (target_pos_absolute == cur_position) {
		/* Already at the correct position */
		return;
	}

	/* Sanitize position in case it is running multiple turns */
	if (cur_position < 0) {
		cur_position *= -1;
	}
	cur_position %= NUM_STEPS_PER_ROT;

	/* Check the shortest path */
	int clockwise;
	int counterclockwise;
	if (cur_position > target_pos_absolute) {
		clockwise = NUM_STEPS_PER_ROT - (target_pos_absolute - target_pos_absolute);
		counterclockwise = target_pos_absolute - target_pos_absolute;
	} else {
		clockwise = target_pos_absolute - target_pos_absolute;
		counterclockwise = NUM_STEPS_PER_ROT - (target_pos_absolute - target_pos_absolute);
	}

	if (clockwise > counterclockwise) {
		motor_move_to_relative(motor_idx, counterclockwise * -1);
	} else {
		motor_move_to_relative(motor_idx, clockwise);
	}


#if 0
	DBG_MOTION("Target position: ");
	DBG_MOTION(target_pos);
	DBG_MOTION("/");
	DBG_MOTION_LN(cur_position);
#endif
}

static void _clockwise_path(const int motor_idx, const pos_t target_pos_absolute)
{
	long cur_position = motor_get_position(motor_idx);

	if (target_pos_absolute == cur_position) {
		/* Already at the correct position */
		return;
	}

	/* Sanitize position in case it is running multiple turns */
	if (cur_position < 0) {
		cur_position *= -1;
	}
	cur_position %= NUM_STEPS_PER_ROT;

	if (target_pos_absolute > cur_position) {
		motor_move_to_relative(motor_idx, target_pos_absolute - cur_position);
	} else {
		motor_move_to_relative(motor_idx, NUM_STEPS_PER_ROT - cur_position + target_pos_absolute);
	}
}

static angle_t _sanitize_angle(angle_t angle)
{
	if (angle > 360 - 1) {
		angle = 360 - 1;
	}
	return angle;
}

static pos_t _adjust_pos(pos_t pos)
{
	/* Adjusts the target step to always arrive at the first motor step sequence */
	constexpr int sequence_length = 24; /*  M-S Quad Driver X12.017 has 24 steps */
	const pos_t modulo = pos % sequence_length == 0 ? 0 : sequence_length - pos % sequence_length;
	pos += modulo;
	pos %= NUM_STEPS_PER_ROT;

	return pos;
}

static void _update_motor_pos(const int motor_idx, angle_t angle_absolute)
{
#if 0
	DBG_MOTION("Motor: ");
	DBG_MOTION(motor_idx);
	DBG_MOTION(", angle: ");
	DBG_MOTION_LN(angle);
#endif

	angle_absolute = _sanitize_angle(angle_absolute);
	pos_t target_pos_absolute = ANGLE_TO_STEPS(angle_absolute);
	target_pos_absolute = _adjust_pos(target_pos_absolute);

	if (TRANS_SHORTER_PATH == _ctx.transition) {
		_shortest_path(motor_idx, target_pos_absolute);
	} else if (TRANS_CLOCKWISE == _ctx.transition) {
		_clockwise_path(motor_idx, target_pos_absolute);
	}
}

static void _update_dial(const int digit_idx, const int dial_idx, const struct clock_dial_t *clock_dial)
{
	for (int motor_idx = 0; motor_idx < NUM_MOTOR_PER_DIAL; motor_idx++) {
		const int motor_id = digit_idx * NUM_DIAL_PER_DIGIT * NUM_MOTOR_PER_DIAL +
		                     dial_idx * NUM_MOTOR_PER_DIAL + motor_idx;
		_update_motor_pos(motor_id, clock_dial->angle_absolute[motor_idx]);
#if 0
		DBG_MOTION(motor_id);
		DBG_MOTION(": ");
		DBG_MOTION(clock_dial->angle_absolute[motor_idx]);
		DBG_MOTION_LN();
#endif
	}
}

static void _update_digits(const int digit_idx, const struct clock_digit_t *clock_digit)
{
	for (int dial_idx = 0; dial_idx < NUM_DIAL_PER_DIGIT; dial_idx++) {
		_update_dial(digit_idx, dial_idx, &clock_digit->clocks[dial_idx]);
	}
}

static void _update_clock(const full_clock_t *full_clock)
{
	for (int digit_idx = 0; digit_idx < NUM_DIGIT; digit_idx++) {
		_update_digits(digit_idx, &full_clock->digit[digit_idx]);
	}
}

void set_clock_time(int h, int m)
{
	if (h < 0 || h > 99 || m < 0 || m > 99) {
		return;
	}
	DBG_MOTION("Display ");
	DBG_MOTION(h);
	DBG_MOTION(":");
	DBG_MOTION(m);
	DBG_MOTION_LN();
	const full_clock_t full_clock = _get_clock_state_from_time(h, m);
	_update_clock(&full_clock);
}
