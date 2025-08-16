#include "motor_motion.h"
#include "AccelStepper.h"
#include "cfg.hpp"
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

static constexpr int MOTOR_MAX_SPEED = 1500;
static constexpr int MOTOR_ACC = 100;

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

// Define driver and motor IDs with more descriptive names
enum DriverId : uint8_t {
	M1,
	M2,
	M3,
	M4,
	M5,
	M6,
	M7,
	M8,
	M9,
	M10,
	M11,
	M12,
	M13,
	M14,
	M15,
	M16,
	M17,
	M18,
	M19,
	M20,
	M21,
	M22,
	M23,
	M24,
};

enum MotorId : uint8_t {
	MOTOR_0,
	MOTOR_1,
	MOTOR_2,
	MOTOR_3,
	MOTOR_4,
	MOTOR_5,
	MOTOR_6,
	MOTOR_7,
	MOTOR_8,
	MOTOR_9,
	MOTOR_10,
	MOTOR_11,
	MOTOR_12,
	MOTOR_13,
	MOTOR_14,
	MOTOR_15,
	MOTOR_16,
	MOTOR_17,
	MOTOR_18,
	MOTOR_19,
	MOTOR_20,
	MOTOR_21,
	MOTOR_22,
	MOTOR_23,
	MOTOR_24,
	MOTOR_25,
	MOTOR_26,
	MOTOR_27,
	MOTOR_28,
	MOTOR_29,
	MOTOR_30,
	MOTOR_31,
	MOTOR_32,
	MOTOR_33,
	MOTOR_34,
	MOTOR_35,
	MOTOR_36,
	MOTOR_37,
	MOTOR_38,
	MOTOR_39,
	MOTOR_40,
	MOTOR_41,
	MOTOR_42,
	MOTOR_43,
	MOTOR_44,
	MOTOR_45,
	MOTOR_46,
	MOTOR_47,
};

/**
 * @brief Maps a logical motor and driver combination to a physical motor index.
 *
 * The mapping logic is based on a regular pattern:
 * - Drivers are grouped in pairs: (M1, M2), (M3, M4), ..., (M23, M24).
 * - Each pair corresponds to a 4-slot block of physical motor indices.
 * - These blocks are ordered in descending order starting from (NUM_MOTORS - 4).
 * - For each pair:
 *   - The first driver in the pair maps to the first two slots in the block:
 *       even (lower) motors → offset 0 (D), odd (upper) motors → offset 1 (A)
 *   - The second driver in the pair maps to the next two slots:
 *       even → offset 2 (C), odd → offset 3 (B)
 *
 * @param motor The motor ID (even = lower, odd = upper).
 * @param driver The driver to which the motor is assigned (M1–M24).
 * @return The corresponding physical motor index, or MAP_ERROR if invalid.
 */
static constexpr int mapMotorToPhysical(const MotorId motor, const DriverId driver)
{
	constexpr int MAP_ERROR = -1;
	if (driver > M24) {
		return MAP_ERROR;
	}

	const bool is_lower_motor = motor % 2 == 0;
	const int driver_index = driver;
	const int block = driver_index / 2;
	const int block_base = (NUM_MOTORS - 4) - (block * 4);

	const bool is_first_driver_in_pair = (driver_index % 2 == 0);

	if (is_first_driver_in_pair) {
		return block_base + (is_lower_motor ? 0 : 1); // D or A
	} else {
		return block_base + (is_lower_motor ? 2 : 3); // C or B
	}
}


// Test of the mapping function
static_assert(mapMotorToPhysical(MOTOR_0, M1) == MOTOR_44);
static_assert(mapMotorToPhysical(MOTOR_1, M1) == MOTOR_45);
static_assert(mapMotorToPhysical(MOTOR_2, M2) == MOTOR_46);
static_assert(mapMotorToPhysical(MOTOR_3, M2) == MOTOR_47);
static_assert(mapMotorToPhysical(MOTOR_30, M23) == MOTOR_0);
static_assert(mapMotorToPhysical(MOTOR_31, M23) == MOTOR_1);
static_assert(mapMotorToPhysical(MOTOR_30, M24) == MOTOR_2);
static_assert(mapMotorToPhysical(MOTOR_31, M24) == MOTOR_3);

constexpr std::array<DriverId, NUM_MOTORS> MOTOR_DRIVER_ASSIGNMENT = []() {
	std::array<DriverId, NUM_MOTORS> arr{};
	// Digit 1
	arr.at(47) = M14;
	arr.at(46) = M14;
	arr.at(45) = M12;
	arr.at(44) = M12;
	arr.at(43) = M13;
	arr.at(42) = M13;
	arr.at(41) = M11;
	arr.at(40) = M11;
	arr.at(39) = M15;
	arr.at(38) = M15;
	arr.at(37) = M10;
	arr.at(36) = M10;

	// Digit 2
	arr.at(35) = M9;
	arr.at(34) = M9;
	arr.at(33) = M16;
	arr.at(32) = M16;
	arr.at(31) = M8;
	arr.at(30) = M8;
	arr.at(29) = M17;
	arr.at(28) = M17;
	arr.at(27) = M7;
	arr.at(26) = M7;
	arr.at(25) = M18;
	arr.at(24) = M18;

	// Digit 3
	arr.at(23) = M6;
	arr.at(22) = M6;
	arr.at(21) = M19;
	arr.at(20) = M19;
	arr.at(19) = M5;
	arr.at(18) = M5;
	arr.at(17) = M3;
	arr.at(16) = M3;
	arr.at(15) = M21;
	arr.at(14) = M21;
	arr.at(13) = M20;
	arr.at(12) = M20;

	// Digit 4
	arr.at(11) = M22;
	arr.at(10) = M22;
	arr.at(9) = M2;
	arr.at(8) = M2;
	arr.at(7) = M4;
	arr.at(6) = M4;
	arr.at(5) = M1;
	arr.at(4) = M1;
	arr.at(3) = M24;
	arr.at(2) = M24;
	arr.at(1) = M23;
	arr.at(0) = M23;

	return arr;
}();

constexpr std::array<int, NUM_MOTORS> MOTOR_MAPPING = []() {
	std::array<int, NUM_MOTORS> arr{};
	for (int i = 0; i < NUM_MOTORS; ++i) {
		arr[i] = mapMotorToPhysical(static_cast<MotorId>(i), MOTOR_DRIVER_ASSIGNMENT[i]);
	}
	return arr;
}();

static void set_motor_bits_(int motor_id, const int sequence)
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
	motor_id = static_cast<int>(MOTOR_MAPPING.at(motor_id));
	if (motor_id < 0 || motor_id >= NUM_MOTORS) {
		return;
	}
	const int motor_num = motor_id % NUM_MOTORS_PER_SHIFT_REG;
	steps_.at(motor_id / NUM_MOTORS_PER_SHIFT_REG) &= ~(0b11 << (motor_num * 2));
	steps_.at(motor_id / NUM_MOTORS_PER_SHIFT_REG) |= sequence << (motor_num * 2);
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
	if (_motor_id % 2 == 0) {
		sequence = _direction ? 0b01 : 0b00;
	} else {
		sequence = _direction ? 0b00 : 0b01;
	}

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

void motor_test(const int motor_id)
{
	const int move = motor_id % 2 == 0 ? -NUM_STEPS_PER_ROT : NUM_STEPS_PER_ROT;
	motors_.at(motor_id).move(move);
	// for (auto &motor : motors_) {
	// 	motor.move(NUM_STEPS_PER_ROT/2);
	// }
	// motors_.at(motor_id).move(NUM_STEPS_PER_ROT);
}
