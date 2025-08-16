#include "wave_animation.hpp"
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

void WaveAnimation::start()
{
	state_ = AnimationState::RUNNING;
	start_time_ms_ = millis();
	current_phase_ = WavePhase::INITIALIZING;
	current_rotating_motor_ = 0;
	last_motor_start_time_ = 0;

	DBG_ANIM_LN("Starting wave animation - moving all hands to 0°");

	// Move all hands to 0° position (12 o'clock)
	for (int motor_id = 0; motor_id < NUM_MOTORS; motor_id++) {
		motor_move_to_absolute(motor_id, 0);
	}
}

void WaveAnimation::update(uint32_t current_time_ms)
{
	if (state_ != AnimationState::RUNNING) {
		return;
	}

	switch (current_phase_) {
	case WavePhase::INITIALIZING:
		// Wait for all motors to reach 0° position
		if (are_motors_idle()) {
			current_phase_ = WavePhase::ROTATING;
			last_motor_start_time_ = current_time_ms;
			DBG_ANIM_LN("Initialization complete - starting sequential rotations");
		}
		break;

	case WavePhase::ROTATING:
		// Check if it's time to start next motor rotation
		if (current_rotating_motor_ < NUM_MOTORS &&
		    current_time_ms - last_motor_start_time_ >= DELAY_BETWEEN_MOTORS_MS) {
			DBG_ANIM("Starting rotation for motor ");
			DBG_ANIM_LN(current_rotating_motor_);

			// Start one complete rotation (360°)
			motor_move_to_absolute(current_rotating_motor_, NUM_STEPS_PER_ROT);

			current_rotating_motor_++;
			last_motor_start_time_ = current_time_ms;
		}

		// Check if animation is complete (all motors started and all are idle)
		if (current_rotating_motor_ >= NUM_MOTORS && are_motors_idle()) {
			DBG_ANIM_LN("Wave animation completed - all rotations finished");
			stop();
		}
		break;
	}
}

void WaveAnimation::stop()
{
	state_ = AnimationState::FINISHED;
	DBG_ANIM_LN("Wave animation finished");
}
