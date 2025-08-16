#include "sync_rotation_animation.hpp"
#include "../motor/motion.hpp"
#include "../motor/motor_motion.h"
#include "cfg.hpp"

#ifdef DEBUG_ANIMATION
#define DBG_ANIM(...)    Serial.print(__VA_ARGS__)
#define DBG_ANIM_LN(...) Serial.println(__VA_ARGS__)
#else
#define DBG_ANIM(...)
#define DBG_ANIM_LN(...)
#endif

void SyncRotationAnimation::start()
{
	state_ = AnimationState::RUNNING;
	start_time_ms_ = millis();
	rotation_started_ = false;

	DBG_ANIM_LN("Starting sync rotation animation");

	// Phase 1: Move all hands to 0h30 position (alternating 0° and 180°)
	for (int motor_id = 0; motor_id < NUM_MOTORS; motor_id++) {
		static constexpr uint32_t DEGREES_PER_ROTATION = 360;

		// Alternate between 0° (minute hand at 12) and 180° (hour hand at 6)
		// This creates the 6:30 pattern where hands point in opposite directions
		const uint16_t target_angle = (motor_id % 2 == 0) ? 0 : TARGET_ANGLE;

		const auto target_steps = static_cast<uint16_t>(
		    (static_cast<uint32_t>(target_angle) * NUM_STEPS_PER_ROT) / DEGREES_PER_ROTATION);
		motor_move_to_absolute(motor_id, target_steps);
	}
}

void SyncRotationAnimation::update(const uint32_t current_time_ms)
{
	if (state_ != AnimationState::RUNNING) {
		return;
	}

	const uint32_t elapsed_time = current_time_ms - start_time_ms_;

	// Check if animation duration exceeded (safety timeout)
	if (elapsed_time >= duration_ms_) {
		DBG_ANIM_LN("Animation timeout reached - forcing stop");
		stop();
		return;
	}

	// Phase 1: Wait for all motors to reach alignment position
	if (!rotation_started_) {
		if (!are_motors_idle()) {
			return; // Still moving to alignment position
		}

		// Phase 2: Start synchronized rotation
		rotation_started_ = true;
		DBG_ANIM_LN("Starting synchronized rotation");

		// Calculate steps for one full rotation
		for (int motor_id = 0; motor_id < NUM_MOTORS; motor_id++) {
			motor_move_to_relative(motor_id, NUM_STEPS_PER_ROT * 2);
		}
	} else {
		// Phase 2: Check if rotation is complete
		if (are_motors_idle()) {
			DBG_ANIM_LN("All motors finished rotation - animation complete");
			stop();
		}
	}
}

void SyncRotationAnimation::stop()
{
	state_ = AnimationState::FINISHED;
	DBG_ANIM_LN("Sync rotation animation finished");
}
