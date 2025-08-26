#include "sync_rotation_animation.hpp"
#include "cfg.hpp"
#include "motor/motion.hpp"
#include "motor/motor_helper.h"
#include "motor/motor_motion.h"

void SyncRotationAnimation::start()
{
	state_ = AnimationState::RUNNING;
	start_time_ms_ = millis();
	current_phase_ = AnimationPhase::ALIGNING;
	current_rotating_motor_ = 0;
}

void SyncRotationAnimation::update(const uint32_t current_time_ms)
{
	if (state_ != AnimationState::RUNNING) {
		return;
	}

	const uint32_t elapsed_time = current_time_ms - start_time_ms_;

	// Check if animation duration exceeded (safety timeout)
	if (elapsed_time >= duration_ms_) {
		stop();
		return;
	}

	switch (current_phase_) {
	case AnimationPhase::ALIGNING:
		if (constexpr int DELAY_BETWEEN_MOTORS_MS = 2000;
		    current_rotating_motor_ < NUM_MOTORS &&
		    current_time_ms - last_motor_start_time_ >= DELAY_BETWEEN_MOTORS_MS) {
			for (int i = 0; i < MOTOR_NUMBER_ROWS * NUM_MOTOR_PER_CIRCLE; i++) {
				constexpr uint16_t TARGET_ANGLE = 180; // 0h30 position
				// The first column and last column will have a diff og 90°
				constexpr int angle = 90 / MOTOR_NUMBER_COLUMNS;
				const int angle_position =
				    (current_rotating_motor_ / MOTOR_NUMBER_ROWS) * angle;

				// Alternate between 0° (minute hand at 12) and 180° (hour hand at 6)
				const uint16_t target_angle =
				    (current_rotating_motor_ % 2 == 0) ? 0 : TARGET_ANGLE;

				const auto target_steps = angleToSteps(target_angle + angle_position);
				motor_move_to_absolute(current_rotating_motor_, target_steps);

				motor_set_acceleration(current_rotating_motor_, 300);

				current_rotating_motor_++;
			}

			last_motor_start_time_ = current_time_ms;
		}
		if (current_rotating_motor_ >= NUM_MOTORS && are_motors_idle()) {
			current_phase_ = AnimationPhase::ROTATING;
		}
		break;
	case AnimationPhase::ROTATING:
		for (int motor_id = 0; motor_id < NUM_MOTORS; motor_id++) {
			motor_move_to_relative(motor_id, angleToSteps(360 * 2));
			motor_set_max_speed(motor_id, MAX_MOTOR_SPEED);
			motor_set_acceleration(motor_id, 100);
		}
		current_phase_ = AnimationPhase::WAIT_ENDING;
		break;
	case AnimationPhase::WAIT_ENDING:
		if (are_motors_idle()) {
			stop();
		}
	}
}

void SyncRotationAnimation::stop()
{
	state_ = AnimationState::FINISHED;
}
