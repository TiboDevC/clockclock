#include <Arduino.h>
#include <stdint.h>

#include "shift_register.h"

#define NUM_MOTORS           48
#define NUM_STEPS_PER_ROT    (4096)
#define DELAY_FACTOR         5
#define DELAY_OFFSET         1500 /* min delay in micro second to switch to next motor sequence */
#define ANGLE_TO_STEPS(target_angle) \
	((uint16_t) (((uint32_t) target_angle * NUM_STEPS_PER_ROT) / (uint32_t) 360))

#define INITIAL_DELAY        200
#define INITIAL_SPEED        200
#define INITIAL_ACCELERATION 1

typedef uint16_t pos_t;

enum direction_t : uint8_t {
	DIRECTION_CLOCKWISE,
	DIRECTION_COUNTERCLOCKWISE,
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
    0b1000,
    0b0100,
    0b0010,
    0b0001,
    0b0000,
};

static struct {
	uint8_t acceleration;
	uint8_t speed;
	enum transition_t transition;
} _ctx = {.acceleration = INITIAL_ACCELERATION, .speed = INITIAL_SPEED, .transition = TRANS_SHORTER_PATH};

static struct motor_t _motors[NUM_MOTORS] = {};

static uint8_t _steps[NUM_MOTORS] = {0};

static void _print_motor(const int motor_id, const motor_t *motor)
{
	Serial.print(motor_id);
	Serial.print(": ");
	Serial.print(motor->step_remaining);
	Serial.print("/");
	Serial.print(motor->current_pos);
	Serial.print(", dir: ");
	Serial.print(motor->direction);
	Serial.print(", delay: ");
	Serial.print(motor->delay_us);
	Serial.print("/");
	Serial.print(motor->delay_target_us);
	Serial.print(", last_us: ");
	Serial.print(motor->last_delay);
	Serial.print(", step: ");
	Serial.print(motor->step);
	Serial.println();
}

static void _set_motor_bits(int motor_id, int step_id)
{
	_steps[motor_id] = _motor_steps[step_id];
}

static void _update_pos(struct motor_t *motor)
{
	if (0 == motor->step_remaining) {
		/* No step remaining, motor is off */
		motor->step = MOTOR_STEP_OFF;
	} else {
		motor->step_remaining--;

		if (DIRECTION_CLOCKWISE == motor->direction) {
			motor->current_pos++;
			motor->step = (enum motor_step_t)((uint8_t) (motor->step + 1) % MOTOR_STEP_OFF);
		} else {
			motor->current_pos--;
			motor->step = (enum motor_step_t)((uint8_t) (motor->step - 1) % MOTOR_STEP_OFF);
		}

		motor->current_pos %= NUM_STEPS_PER_ROT;
	}
}

static int _check_delay(const struct motor_t *motor, unsigned long time_us)
{
	if (0 != motor->step_remaining)
		if (motor->last_delay > time_us ||
		    time_us - motor->last_delay >
		        (((unsigned long) motor->delay_us * DELAY_FACTOR) + DELAY_OFFSET)) {
			return 0;
		}
	return -1;
}

static void _update_delay(struct motor_t *motor)
{
	if (motor->delay_target_us > motor->delay_us) {
		motor->delay_us -= _ctx.acceleration;
	} else if (motor->delay_target_us < motor->delay_us) {
		motor->delay_us += _ctx.acceleration;
	}
}

void step_motors()
{
	const unsigned long time_us = micros();

	for (int motor_id = 0; motor_id < NUM_MOTORS; motor_id++) {
		if (0 == _check_delay(&_motors[motor_id], time_us)) {
			_update_pos(&_motors[motor_id]);
			_update_delay(&_motors[motor_id]);
			_set_motor_bits(motor_id, _motors[motor_id].step);
			_motors[motor_id].last_delay = time_us;
		}

		_print_motor(motor_id, &_motors[motor_id]);
	}

	ctrl_motors(_steps, NUM_MOTORS);
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

typeof(_ctx.speed) get_speed()
{
	return _ctx.speed;
}

void set_speed(typeof(_ctx.speed) value)
{
	_ctx.speed = value;
}

typeof(_ctx.acceleration) get_acceleration()
{
	return _ctx.acceleration;
}

void set_acceleration(typeof(_ctx.acceleration) value)
{
	_ctx.acceleration = value;
}

static struct full_clock_t _get_clock_state_from_time(int h, int m)
{
	const int d0 = h / 10;
	const int d1 = h - d0 * 10;
	const int d2 = m / 10;
	const int d3 = m - d2 * 10;
	Serial.print("Set time: ");
	Serial.print(h);
	Serial.print(":");
	Serial.println(m);
	struct full_clock_t clock_state = {_digits[d0], _digits[d1], _digits[d2], _digits[d3]};
	return clock_state;
}

static void _shortest_path(const int needle_idx, const pos_t target_pos)
{
	motor_t *motor = &_motors[needle_idx];

	if (target_pos == motor->current_pos) {
		/* Already at the correct position */
		return;
	} else {
		/* Figure out the shortest path */
		const pos_t clockwise = target_pos;
		const pos_t counterclockwise = NUM_STEPS_PER_ROT - target_pos;

		if (clockwise < counterclockwise) {
			motor->step_remaining = clockwise;
			motor->direction = DIRECTION_CLOCKWISE;
		} else {
			motor->step_remaining = counterclockwise;
			motor->direction = DIRECTION_COUNTERCLOCKWISE;
		}

		/* Update the speed */
		motor->delay_target_us = INITIAL_DELAY;
		motor->last_delay = 0;
	}

	Serial.println(target_pos);
	_print_motor(needle_idx, motor);
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
	pos_t modulo = 4 - pos % 4; /*  4 = number of steps in the motor sequence */
	pos += modulo;
	pos %= NUM_STEPS_PER_ROT;

	return pos;
}

static void _update_needle(const int needle_idx, angle_t angle)
{
	angle = _sanitize_angle(angle);
	Serial.print("angle: ");
	Serial.println(angle);
	pos_t target_pos = ANGLE_TO_STEPS(angle);
	target_pos = _adjust_pos(target_pos);
	Serial.print("pos: ");
	Serial.println(target_pos);

	if (TRANS_SHORTER_PATH == _ctx.transition) {
		_shortest_path(needle_idx, target_pos);
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

void set_clock_time(int h, int m)
{
	if (h < 0 || h > 99 || m < 0 || m > 99) {
		return;
	}
	const struct full_clock_t full_clock = _get_clock_state_from_time(h, m);
	_update_clock(&full_clock);
}
