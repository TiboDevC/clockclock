#include <stdint.h>

#include <Arduino.h>

#include "cfg.hpp"
#include "shift_register.h"

#ifdef DEBUG_MOTION
#define DBG_MOTION(...)    Serial.print(__VA_ARGS__)
#define DBG_MOTION_LN(...) Serial.println(__VA_ARGS__)
#else
#define DBG_MOTION(...)
#define DBG_MOTION_LN(...)
#endif

#define NUM_STEPS_PER_ROT (4096)
#define DELAY_FACTOR      10
#define DELAY_OFFSET      1500 /* min delay in micro second to switch to next motor sequence */
#define ANGLE_TO_STEPS(target_angle) \
	((uint16_t) (((uint32_t) target_angle * NUM_STEPS_PER_ROT) / (uint32_t) 360))
#define DELAY_TO_US(delay) (((unsigned long) delay * DELAY_FACTOR) + DELAY_OFFSET)

#define MAX_DELAY            255
#define INITIAL_DELAY        255
#define INITIAL_SPEED        10
#define INITIAL_ACCELERATION 1

typedef uint16_t pos_t;

enum direction_t : uint8_t {
	DIRECTION_CLOCKWISE,
	DIRECTION_COUNTERCLOCKWISE,
};

enum motion_mode_t : uint8_t {
	MOTION_NORMAL,
	MOTION_CALIB,
};

enum motor_step_t : uint8_t {
	MOTOR_STEP_0,
	MOTOR_STEP_1,
	MOTOR_STEP_2,
	MOTOR_STEP_3,
	MOTOR_STEP_OFF,
	MOTOR_STEP_MAX
};

enum transition_t : uint8_t {
	TRANS_SHORTER_PATH,
	TRANS_CLOCKWISE,
};

struct motor_t {
	unsigned long last_delay;
	pos_t current_pos;
	pos_t step_remaining;
	uint8_t delay_us;
	uint8_t delay_target_us;
	enum direction_t direction;
	enum motor_step_t step;
};

static const int _motor_steps[MOTOR_STEP_MAX] = {
    0b1000, /* MOTOR_STEP_0 */
    0b0100, /* MOTOR_STEP_1 */
    0b0010, /* MOTOR_STEP_2 */
    0b0001, /* MOTOR_STEP_3 */
    0b0000, /* MOTOR_STEP_OFF */
};

static struct {
	uint8_t acceleration;
	uint8_t speed;
	enum transition_t transition;
	enum motion_mode_t motion_mode;
} _ctx = {.acceleration = INITIAL_ACCELERATION,
          .speed = INITIAL_SPEED,
          .transition = TRANS_SHORTER_PATH,
          .motion_mode = MOTION_NORMAL};

static struct motor_t _motors[NUM_MOTORS] = {};

static uint8_t _steps[SHIFT_REG_SIZE] = {0}; /* 4 bits per motors */

static void _print_motor(const int motor_id, const motor_t *motor)
{
	DBG_MOTION(motor_id);
	DBG_MOTION(": ");
	DBG_MOTION(motor->step_remaining);
	DBG_MOTION("/");
	DBG_MOTION(motor->current_pos);
	DBG_MOTION(", dir: ");
	DBG_MOTION(motor->direction);
	DBG_MOTION(", delay: ");
	DBG_MOTION(motor->delay_us);
	DBG_MOTION("/");
	DBG_MOTION(motor->delay_target_us);
	DBG_MOTION("(");
	DBG_MOTION(DELAY_TO_US(motor->delay_us));
	DBG_MOTION("ms)");
	DBG_MOTION(", last_us: ");
	DBG_MOTION(motor->last_delay);
	DBG_MOTION(", step: ");
	DBG_MOTION(motor->step);
	DBG_MOTION_LN("");
}

static void _set_motor_bits(int motor_id, int step_id)
{
	_steps[motor_id / 2] &= ~(0xF << ((motor_id % 2) * 4));
	_steps[motor_id / 2] |= _motor_steps[step_id] << ((motor_id % 2) * 4);
}

static void _update_pos(struct motor_t *motor)
{
	if (0 == motor->step_remaining) {
		/* No step remaining, motor is off */
		motor->step = MOTOR_STEP_OFF;
	} else {
		motor->step_remaining--;

		if (DIRECTION_CLOCKWISE == motor->direction) {
			if (MOTION_CALIB != _ctx.motion_mode) {
				motor->current_pos++;
			}
			motor->step = (enum motor_step_t)((uint8_t) (motor->step + 1) % MOTOR_STEP_OFF);
		} else {
			if (MOTION_CALIB != _ctx.motion_mode) {
				motor->current_pos--;
			}
			motor->step = (enum motor_step_t)((uint8_t) (motor->step - 1) % MOTOR_STEP_OFF);
		}

		motor->current_pos %= NUM_STEPS_PER_ROT;
	}
}

static int _check_delay(const struct motor_t *motor, unsigned long time_us)
{
	if (motor->last_delay > time_us || time_us - motor->last_delay > DELAY_TO_US(motor->delay_us)) {
#if 0
			DBG_MOTION("Delay: ");
			DBG_MOTION(motor->delay_us);
			DBG_MOTION(", ");
			DBG_MOTION_LN(DELAY_TO_US(motor->delay_us));
#endif
		return 0;
	}
	return -1;
}

static void _update_delay(struct motor_t *motor)
{
	if (motor->step_remaining / _ctx.acceleration < INITIAL_DELAY) {
		/* Update target if it's time to decelerate */
		motor->delay_target_us = INITIAL_DELAY;
	} else {
		/* Otherwise, set it to target speed */
		motor->delay_target_us = _ctx.speed;
	}

	if (motor->delay_target_us > motor->delay_us) {
		if (MAX_DELAY - _ctx.acceleration > motor->delay_us) {
			motor->delay_us = MAX_DELAY;
		} else {
			motor->delay_us += _ctx.acceleration;
		}
	} else if (motor->delay_target_us < motor->delay_us) {
		if (_ctx.acceleration > motor->delay_us) {
			motor->delay_us = 0;
		} else {
			motor->delay_us -= _ctx.acceleration;
		}
	}
}

void loop_motors()
{
	const unsigned long time_us = micros();

	for (int motor_id = 0; motor_id < NUM_MOTORS; motor_id++) {
		if (0 == _check_delay(&_motors[motor_id], time_us)) {
			_update_pos(&_motors[motor_id]);
			_update_delay(&_motors[motor_id]);
			_set_motor_bits(motor_id, _motors[motor_id].step);
			_motors[motor_id].last_delay = time_us;
		}

		// _print_motor(motor_id, &_motors[motor_id]);
	}

	ctrl_motors(_steps, sizeof(_steps));
}


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

#define NUM_NEEDLE_PER_DIAL 2
#define NUM_DIAL_PER_DIGIT  6
#define NUM_DIGIT           4

typedef uint16_t angle_t;

struct clock_dial_t {
	angle_t angle[NUM_NEEDLE_PER_DIAL];
};

struct clock_digit_t {
	struct clock_dial_t clocks[NUM_DIAL_PER_DIGIT];
};

struct full_clock_t {
	struct clock_digit_t digit[NUM_DIGIT];
};

static const struct clock_digit_t digit_0 = {
    .clocks = {{270, 0}, {270, 90}, {0, 90}, {270, 180}, {270, 90}, {180, 90}}};

static const struct clock_digit_t digit_1 = {
    .clocks = {{225, 225}, {225, 225}, {225, 225}, {270, 270}, {270, 90}, {90, 90}}};

static const struct clock_digit_t digit_2 = {
    .clocks = {{0, 0}, {270, 0}, {90, 0}, {180, 270}, {90, 180}, {180, 180}}};

static const struct clock_digit_t digit_3 = {
    .clocks = {{0, 0}, {0, 0}, {0, 0}, {180, 270}, {180, 90}, {180, 90}}};

static const struct clock_digit_t digit_4 = {
    .clocks = {{270, 270}, {90, 0}, {225, 225}, {270, 270}, {270, 90}, {90, 90}}};

static const struct clock_digit_t digit_5 = {
    .clocks = {{270, 0}, {90, 0}, {0, 0}, {180, 180}, {270, 180}, {90, 180}}};

static const struct clock_digit_t digit_6 = {
    .clocks = {{270, 0}, {270, 90}, {90, 0}, {180, 180}, {270, 180}, {90, 180}}};

static const struct clock_digit_t digit_7 = {
    .clocks = {{0, 0}, {225, 225}, {225, 225}, {270, 180}, {270, 90}, {90, 90}}};

static const struct clock_digit_t digit_8 = {
    .clocks = {{270, 0}, {90, 0}, {90, 0}, {270, 180}, {90, 180}, {90, 180}}};

static const struct clock_digit_t digit_9 = {
    .clocks = {{270, 0}, {0, 90}, {0, 0}, {270, 180}, {270, 90}, {90, 180}}};

static const struct clock_digit_t digit_null = {
    .clocks = {{270, 270}, {270, 270}, {270, 270}, {270, 270}, {270, 270}, {270, 270}}};

static const struct clock_digit_t digit_I = {
    .clocks = {{270, 90}, {270, 90}, {270, 90}, {270, 90}, {270, 90}, {270, 90}}};

static const struct clock_digit_t digit_fun = {.clocks = {
						   {225, 45},
						   {225, 45},
						   {225, 45},
						   {225, 45},
						   {225, 45},
						   {225, 45},
					       }};

static const clock_digit_t _digits[10] = {
    digit_0, digit_1, digit_2, digit_3, digit_4, digit_5, digit_6, digit_7, digit_8, digit_9};

void set_speed(uint8_t value)
{
	_ctx.speed = value;
}

void set_acceleration(uint8_t value)
{
	_ctx.acceleration = value;
}

static struct full_clock_t _get_clock_state_from_time(int h, int m)
{
	DBG_MOTION("Set time: ");
	DBG_MOTION(h);
	DBG_MOTION(":");
	DBG_MOTION_LN(m);

	const int d0 = h / 10;
	const int d1 = h - d0 * 10;
	const int d2 = m / 10;
	const int d3 = m - d2 * 10;

	struct full_clock_t clock_state = {_digits[d0], _digits[d1], _digits[d2], _digits[d3]};
	return clock_state;
}

static void _shortest_path(const int needle_idx, const pos_t target_pos)
{
	motor_t *motor = &_motors[needle_idx];

	if (target_pos == motor->current_pos) {
		/* Already at the correct position */
		return;
	}

	/* Figure out the shortest path */
	pos_t clockwise;
	pos_t counterclockwise;
	if (target_pos > motor->current_pos) {
		clockwise = target_pos - motor->current_pos;
		counterclockwise = NUM_STEPS_PER_ROT - (target_pos - motor->current_pos);
	} else {
		clockwise = NUM_STEPS_PER_ROT - (motor->current_pos - target_pos);
		counterclockwise = motor->current_pos - target_pos;
	}

	if (clockwise < counterclockwise) {
		motor->step_remaining = clockwise;
		motor->direction = DIRECTION_CLOCKWISE;
	} else {
		motor->step_remaining = counterclockwise;
		motor->direction = DIRECTION_COUNTERCLOCKWISE;
	}

#if 0
	DBG_MOTION("Target position: ");
	DBG_MOTION(target_pos);
	DBG_MOTION("/");
	DBG_MOTION_LN(motor->current_pos);
#endif
	_print_motor(needle_idx, motor);
}

static void _clockwise_path(const int needle_idx, const pos_t target_pos)
{
	motor_t *motor = &_motors[needle_idx];

	if (target_pos == motor->current_pos) {
		/* Already at the correct position */
		return;
	}

	if (target_pos > motor->step_remaining) {
		motor->step_remaining = target_pos - motor->step_remaining;
	} else {
		motor->step_remaining = NUM_STEPS_PER_ROT - motor->current_pos + target_pos;
	}
	motor->direction = DIRECTION_CLOCKWISE;

	_print_motor(needle_idx, motor);
}

static void _init_motor_timing(struct motor_t *motor)
{
	/* Update the speed */
	motor->delay_us = INITIAL_DELAY;
	motor->delay_target_us = _ctx.speed;
	motor->last_delay = 0;
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
	pos_t modulo = pos % 4 == 0 ? 0 : 4 - pos % 4; /*  4 = number of steps in the motor sequence */
	pos += modulo;
	pos %= NUM_STEPS_PER_ROT;

	return pos;
}

static void _update_needle(const int needle_idx, angle_t angle)
{
#if 0
	DBG_MOTION("Needle: ");
	DBG_MOTION(needle_idx);
	DBG_MOTION(", angle: ");
	DBG_MOTION_LN(angle);
#endif

	angle = _sanitize_angle(angle);
	pos_t target_pos = ANGLE_TO_STEPS(angle);
	target_pos = _adjust_pos(target_pos);

	if (TRANS_SHORTER_PATH == _ctx.transition) {
		_shortest_path(needle_idx, target_pos);
	} else if (TRANS_CLOCKWISE == _ctx.transition) {
		_clockwise_path(needle_idx, target_pos);
	}
}

static void _update_dial(const int digit_idx, const int dial_idx, const struct clock_dial_t *clock_dial)
{
	for (int needle_idx = 0; needle_idx < NUM_NEEDLE_PER_DIAL; needle_idx++) {
		_update_needle(digit_idx * NUM_DIAL_PER_DIGIT * NUM_NEEDLE_PER_DIAL +
		                   dial_idx * NUM_NEEDLE_PER_DIAL + needle_idx,
		               clock_dial->angle[needle_idx]);
	}
}

static void _update_digits(const int digit_idx, const struct clock_digit_t *clock_digit)
{
	for (int dial_idx = 0; dial_idx < NUM_DIAL_PER_DIGIT; dial_idx++) {
		_update_dial(digit_idx, dial_idx, &clock_digit->clocks[dial_idx]);
	}
}

static void _update_clock(const struct full_clock_t *full_clock)
{
	for (int digit_idx = 0; digit_idx < NUM_DIGIT; digit_idx++) {
		_update_digits(digit_idx, &full_clock->digit[digit_idx]);
	}
}

void animation_init()
{
	for (int motor_idx = 0; motor_idx < NUM_MOTORS; motor_idx++) {
		_init_motor_timing(&_motors[motor_idx]);
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
	const struct full_clock_t full_clock = _get_clock_state_from_time(h, m);
	_update_clock(&full_clock);
}

void increment_needle_pos(const int motor_idx, int16_t increment)
{
	if (increment > 0) {
		_motors[motor_idx].step_remaining += increment;
	} else {
		if (increment * -1 > _motors[motor_idx].step_remaining) {
			if (DIRECTION_CLOCKWISE == _motors[motor_idx].direction) {
				_motors[motor_idx].direction = DIRECTION_COUNTERCLOCKWISE;
			} else {
				_motors[motor_idx].direction = DIRECTION_CLOCKWISE;
			}
			_motors[motor_idx].step_remaining =
			    increment + ((int16_t) _motors[motor_idx].step_remaining);
		} else {
			_motors[motor_idx].step_remaining =
			    ((int16_t) _motors[motor_idx].step_remaining) + increment;
		}
	}
}

void motion_mode_set_calib()
{
	_ctx.motion_mode = MOTION_CALIB;

	/* Set all motors to position 0 */
	for (int motor_idx = 0; motor_idx < NUM_MOTORS; motor_idx++) {
		_update_needle(motor_idx, 0);
		_motors[motor_idx].current_pos = 0;
	}
}

void motion_mode_set_normal()
{
	_ctx.motion_mode = MOTION_NORMAL;
}

void motion_set_motor_neutral()
{
	/* Set all motors to position 0 */
	for (int motor_idx = 0; motor_idx < NUM_MOTORS; motor_idx++) {
		_update_needle(motor_idx, 0);
	}
}
