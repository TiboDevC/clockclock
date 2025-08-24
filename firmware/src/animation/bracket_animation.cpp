#include "bracket_animation.h"

#include "cfg.hpp"
#include "motor/motion.hpp"
#include "motor/motor_helper.h"
#include "motor/motor_motion.h"

void BracketAnimation::start()
{
	state_ = AnimationState::RUNNING;
	start_time_ms_ = millis();
	rotation_started_ = false;

	for (int motor_id = 0; motor_id < NUM_MOTORS; motor_id++) {
		const int column = motor_id / 6;

		if (column % 2 == 0) {
			if (motor_id % 2 == 0) {
				motor_move_to_absolute(motor_id, angleToSteps(45));
			} else {
				motor_move_to_absolute(motor_id, angleToSteps(45 + 90));
			}
		} else {
			if (motor_id % 2 == 0) {
				motor_move_to_absolute(motor_id, angleToSteps(180 + 45));
			} else {
				motor_move_to_absolute(motor_id, angleToSteps(180 + 45 + 90));
			}
		}
	}
}

void BracketAnimation::update(const uint32_t current_time_ms)
{
	if (state_ != AnimationState::RUNNING) {
		return;
	}

	const uint32_t elapsed_time = current_time_ms - start_time_ms_;

	if (elapsed_time >= duration_ms_) {
		stop();
		return;
	}

	if (!rotation_started_) {
		if (are_motors_idle()) {
			for (int motor_id = 0; motor_id < NUM_MOTORS; motor_id++) {
				motor_move_to_relative(motor_id, angleToSteps(360));
			}

			rotation_started_ = true;
		}
	} else {
		if (are_motors_idle()) {
			stop();
		}
	}
}

void BracketAnimation::stop()
{
	state_ = AnimationState::FINISHED;
}
