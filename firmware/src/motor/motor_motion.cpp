#include <cstdint>

#include "AccelStepper.h"
#include "cfg.hpp"
#include "motor_motion.h"
#include "shift_register.h"

#ifdef DEBUG_MOTION
#define DBG_MOTOR_MOTION(...)    Serial.print(__VA_ARGS__)
#define DBG_MOTOR_MOTION_LN(...) Serial.println(__VA_ARGS__)
#else
#define DBG_MOTOR_MOTION(...)
#define DBG_MOTOR_MOTION_LN(...)
#endif

#define ANGLE_TO_STEPS(target_angle) \
	((uint16_t) (((uint32_t) target_angle * NUM_STEPS_PER_ROT) / (uint32_t) 360))
#define STEP_TO_ANGLE(target_step) (((uint32_t) target_step * 360ul) / NUM_STEPS_PER_ROT)

static constexpr int MOTOR_MAX_SPEED = 2000;
static constexpr int MOTOR_ACC = 700;

static constexpr struct {
	uint32_t acceleration;
	uint32_t speed;
} motor_config_ = {.acceleration = MOTOR_ACC, .speed = MOTOR_MAX_SPEED};

static std::array<uint8_t, SHIFT_REG_SIZE> steps_ = {};

enum {
	MOTOR_A,
	MOTOR_B,
	MOTOR_C,
	MOTOR_D,
};

/* Output of 74HCT595 have a specific order: */
enum OUTPUT_74HCT595 {
	OUTPUT_D_74H,
	OUTPUT_A_74H,
	OUTPUT_B_74H,
	OUTPUT_C_74H,
};

static void set_motor_bits_(const int motor_id, const int sequence)
{
	/* 74HCT595 output is:
	 * QA: DIR_D
	 * QB: STEP_D
	 * QC: DIR_A
	 * QD: STEP_A
	 * QE: DIR_B
	 * QF: STEP_B
	 * QG: DIR_D
	 * QH: STEP_C
	 *
	 * SPI is configured as MSB first
	 * */
	const int motor_num = motor_id % NUM_MOTORS_PER_SHIFT_REG;
	switch (motor_num) {
	case MOTOR_A:
		/* Output A */
		steps_.at(motor_id / NUM_MOTORS_PER_SHIFT_REG) &= ~(0b11 << (OUTPUT_A_74H * 2));
		steps_.at(motor_id / NUM_MOTORS_PER_SHIFT_REG) |= sequence << 2 * OUTPUT_A_74H;
		break;
	case MOTOR_B:
		/* Output B */
		steps_.at(motor_id / NUM_MOTORS_PER_SHIFT_REG) &= ~(0b11 << (OUTPUT_B_74H * 2));
		steps_.at(motor_id / NUM_MOTORS_PER_SHIFT_REG) |= sequence << 2 * OUTPUT_B_74H;
		break;
	case MOTOR_C:
		/* Output C */
		steps_.at(motor_id / NUM_MOTORS_PER_SHIFT_REG) &= ~(0b11 << (OUTPUT_C_74H * 2));
		steps_.at(motor_id / NUM_MOTORS_PER_SHIFT_REG) |= sequence << 2 * OUTPUT_C_74H;
		break;
	case MOTOR_D:
		/* Output D */
		steps_.at(motor_id / NUM_MOTORS_PER_SHIFT_REG) &= ~(0b11 << (OUTPUT_D_74H * 2));
		steps_.at(motor_id / NUM_MOTORS_PER_SHIFT_REG) |= sequence << 2 * OUTPUT_D_74H;
		break;
	default:
		break;
	}
}

static void update_direction_()
{
	auto steps = steps_;
	for (auto &step : steps) {
		/* Mask the step instruction, only keep the direction which is the first bit */
		step &= 0b01010101;
	}
	ctrl_motors(steps);
}

static void reset_position_()
{
	for (auto &step : steps_) {
		step &= 0b01010101;
	}
	ctrl_motors(steps_);
}

Motor::Motor()
    : AccelStepper(AccelStepper::DRIVER)
{
	setMaxSpeed(motor_config_.speed);
	setAcceleration(motor_config_.acceleration);
};

void Motor::step1(const long)
{
	uint8_t sequence = 0b00;

	/* Set-up direction, this is the first bit */
	sequence = _direction ? 0b01 : 0b00;

	if (isRunning()) {
		/* Update the step bit, this is the second bit */
		sequence |= 0b10;
	}

	set_motor_bits_(_motor_id, sequence);
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
	/* Reset position in motor lib */
	long new_position = currentPosition() % NUM_STEPS_PER_ROT;
	if (new_position < 0) {
		new_position *= -1;
	}
	setCurrentPosition(new_position);
}

static std::array<Motor, NUM_MOTORS> motors_;

void motor_init()
{
	uint8_t motor_id = 0;
	for (auto &motor : motors_) {
		motor.setMotorId(motor_id);
		motor.setMaxSpeed(motor_config_.speed);
		motor.setAcceleration(motor_config_.acceleration);
		motor_id++;
	}
}

void motor_loop()
{
	for (auto &motor : motors_) {
		if (not motor.run()) {
			motor.disableOutputs();
		}
	}

	/* Step 1) update the direction of the motor - set direction first else get rogue pulses */
	update_direction_();
	/* Step 2) increment the step if needed with a rising edge */
	ctrl_motors(steps_);
	/* Step 3) restore step pin to low */
	reset_position_();
}

void motors_goto_zero()
{
	/* Set all motors to position 0 */
	for (auto &motor : motors_) {
		motor.moveTo(0);
	}
}

void motor_set_0_position()
{
	/* Set all motors to position 0 */
	for (auto &motor : motors_) {
		motor.setCurrentPosition(0);
	}
}

void motor_move_to_relative(const int motor_idx, const int16_t increment)
{
	DBG_MOTOR_MOTION(motor_idx);
	DBG_MOTOR_MOTION(": ");
	DBG_MOTOR_MOTION_LN(increment);
	motors_.at(motor_idx).move(increment);
}

void motor_move_to_absolute(const int motor_idx, const int16_t increment)
{
	motors_.at(motor_idx).moveTo(increment);
}

long motor_get_position(const int motor_idx)
{
	return motors_.at(motor_idx).currentPosition();
}

long motor_distance_to_go(const int motor_idx)
{
	return motors_.at(motor_idx).distanceToGo();
}

void motor_test()
{
	/* Set all motors to position 0 */
	if (not motors_.at(44).isRunning()) {
		motors_.at(44).move(NUM_STEPS_PER_ROT);
	}
	/*
	int i = 0;
	for (auto &motor : _motors) {
	        if ((i + 1) % 2) {
	                motor.move(NUM_STEPS_PER_ROT);
	        }
	        i++;
	}
	*/
}
