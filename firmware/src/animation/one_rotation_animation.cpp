#include "one_rotation_animation.h"

#include "cfg.hpp"
#include "motor/motion.hpp"
#include "motor/motor_motion.h"

void OneRotationAnimation::start()
{
	state_ = AnimationState::RUNNING;
	start_time_ms_ = millis();

	// Move all hands of 360°
	for (int motor_id = 0; motor_id < NUM_MOTORS; motor_id++) {
		motor_move_to_relative(motor_id, NUM_STEPS_PER_ROT);
	}
}

void OneRotationAnimation::update(const uint32_t current_time_ms)
{
	if (state_ != AnimationState::RUNNING) {
		return;
	}

	const uint32_t elapsed_time = current_time_ms - start_time_ms_;

	if (elapsed_time >= duration_ms_ or are_motors_idle()) {
		stop();
	}
}

void OneRotationAnimation::stop()
{
	state_ = AnimationState::FINISHED;
}
