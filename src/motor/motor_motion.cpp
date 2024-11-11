#include <cstdint>

#include "AccelStepper.h"
#include "cfg.hpp"
#include "motor_motion.h"
#include "shift_register.h"

#ifdef DEBUG_MOTION
#define DBG_MOTION(...)    Serial.print(__VA_ARGS__)
#define DBG_MOTION_LN(...) Serial.println(__VA_ARGS__)
#else
#define DBG_MOTION(...)
#define DBG_MOTION_LN(...)
#endif

#define NUM_STEPS_PER_ROT (4096)
#define DELAY_FACTOR      15
#define DELAY_OFFSET      2000 /* min delay in micro second to switch to next motor sequence */
#define ANGLE_TO_STEPS(target_angle) \
	((uint16_t) (((uint32_t) target_angle * NUM_STEPS_PER_ROT) / (uint32_t) 360))
#define STEP_TO_ANGLE(target_step) (((uint32_t) target_step * 360ul) / NUM_STEPS_PER_ROT)
#define DELAY_TO_US(delay)         (((unsigned long) delay * DELAY_FACTOR) + DELAY_OFFSET)

#define MAX_DELAY            255
#define INITIAL_DELAY        255
#define INITIAL_SPEED        500
#define INITIAL_ACCELERATION 200

enum transition_t : uint8_t {
	TRANS_SHORTER_PATH,
	TRANS_CLOCKWISE,
};

static struct {
	uint32_t acceleration;
	uint32_t speed;
	enum transition_t transition;
} _ctx = {.acceleration = INITIAL_ACCELERATION, .speed = INITIAL_SPEED, .transition = TRANS_CLOCKWISE};

static std::array<uint8_t, SHIFT_REG_SIZE> _steps;

static void _set_motor_bits(int motor_id, int sequence)
{
	_steps[motor_id / 2] &= ~(0xF << ((motor_id % 2) * 4));
	_steps[motor_id / 2] |= sequence << ((motor_id % 2) * 4);
}

Motor::Motor()
    : AccelStepper(AccelStepper::FULL4WIRE)
{
	setMaxSpeed(_ctx.speed);
	setAcceleration(_ctx.acceleration);
};

void Motor::step4(const long step)
{
	uint8_t sequence = 0;
	if (isRunning()) {
		switch (step & 0x3) {
		case 0: // 1010
			sequence = 0b0101;
			break;

		case 1: // 0110
			sequence = 0b0110;
			break;

		case 2: // 0101
			sequence = 0b1010;
			break;

		case 3: // 1001
			sequence = 0b1001;
			break;
		}
	}

	_set_motor_bits(_motor_id, sequence);
}

void Motor::step8(const long step)
{
	uint8_t sequence = 0;
	if (isRunning()) {
		switch (step & 0x7) {
		case 0: // 1000
			sequence = 0b0001;
			break;

		case 1: // 1010
			sequence = 0b0101;
			break;

		case 2: // 0010
			sequence = 0b0100;
			break;

		case 3: // 0110
			sequence = 0b0110;
			break;

		case 4: // 0100
			sequence = 0b0010;
			break;

		case 5: // 0101
			sequence = 0b1010;
			break;

		case 6: // 0001
			sequence = 0b1000;
			break;

		case 7: // 1001
			sequence = 0b1001;
			break;
		}
	}

	_set_motor_bits(_motor_id, sequence);
}

void Motor::setMotorId(uint8_t motor_id)
{
	_motor_id = motor_id;
}

void Motor::enableOutputs()
{
	/* Nothing to do */
}

void Motor::disableOutputs()
{
	_set_motor_bits(_motor_id, 0);

	/* Reset position to be sure in motor lib */
	long new_position = currentPosition() % NUM_STEPS_PER_ROT;
	if (new_position < 0) {
		new_position *= -1;
	}
	setCurrentPosition(new_position);
}

static std::array<Motor, NUM_MOTORS> _motors;

void motor_init()
{
	uint8_t motor_id = 0;
	for (auto &motor : _motors) {
		motor.setMotorId(motor_id);
		motor.setMaxSpeed(_ctx.speed);
		motor.setAcceleration(_ctx.acceleration);
		motor_id++;
	}
}

void motor_loop()
{
	for (auto &motor : _motors) {
		if (not motor.run()) {
			motor.disableOutputs();
		}
	}

	ctrl_motors(_steps);
}

void motor_goto_zero()
{
	/* Set all motors to position 0 */
	for (auto &motor : _motors) {
		motor.moveTo(0);
	}
}

void motor_set_0_position()
{
	/* Set all motors to position 0 */
	for (auto &motor : _motors) {
		motor.setCurrentPosition(0);
	}
}

void motor_move_to_relative(const int motor_idx, int16_t increment)
{
	_motors.at(motor_idx).move(increment);
}

void motor_move_to_absolute(const int motor_idx, int16_t increment)
{
	_motors.at(motor_idx).moveTo(increment);
}

long motor_get_position(const int motor_idx)
{
	return _motors.at(motor_idx).currentPosition();
}

long motor_distance_to_go(const int motor_idx)
{
	return _motors.at(motor_idx).distanceToGo();
}
