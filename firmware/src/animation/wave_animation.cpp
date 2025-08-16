#include "wave_animation.hpp"
#include "../motor/motion.hpp"
#include "../motor/motor_motion.h"
#include "cfg.hpp"

#include <cmath>

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
	last_wave_time_ = start_time_ms_;
	current_wave_position_ = 0;
	in_return_phase_ = false;
	return_phase_start_time_ = 0;

	DBG_ANIM_LN("Starting wave animation");
}

void WaveAnimation::update(uint32_t current_time_ms)
{
	if (state_ != AnimationState::RUNNING) {
		return;
	}

	const uint32_t elapsed_time = current_time_ms - start_time_ms_;

	// Check if we should start the return phase
	if (!in_return_phase_ && elapsed_time >= (duration_ms_ - RETURN_PHASE_DURATION)) {
		in_return_phase_ = true;
		return_phase_start_time_ = current_time_ms;
		DBG_ANIM_LN("Starting artistic return phase");

		// Create a beautiful return pattern - all hands slowly return to 0°
		// Start from the last digit and work backwards for a cascading effect
		for (int digit = NUM_DIGITS - 1; digit >= 0; digit--) {
			for (int motor_in_digit = 0; motor_in_digit < NUM_MOTORS_PER_DIGIT;
			     motor_in_digit++) {
				const int motor_id = digit * NUM_MOTORS_PER_DIGIT + motor_in_digit;

				// Add a small delay based on position for cascading effect
				const uint32_t delay_offset =
				    (NUM_DIGITS - 1 - digit) * 100 + motor_in_digit * 20;

				// Schedule movement to 0° with slight delays for artistic effect
				if (elapsed_time >= delay_offset) {
					motor_move_to_absolute(motor_id, 0);
				}
			}
		}
	}

	// Check if return phase is complete
	if (in_return_phase_) {
		if (are_motors_idle()) {
			DBG_ANIM_LN("Artistic return complete - animation finished");
			stop();
		}
		return;
	}

	// Normal wave animation phase
	if (elapsed_time >= duration_ms_) {
		// This shouldn't happen now that we start return phase earlier
		stop();
		return;
	}

	// Create wave effect when motors are ready and interval has passed
	if (current_time_ms - last_wave_time_ >= WAVE_INTERVAL_MS && are_motors_idle()) {
		last_wave_time_ = current_time_ms;

		// Create wave pattern across all digits
		for (int digit = 0; digit < NUM_DIGITS; digit++) {
			for (int motor_in_digit = 0; motor_in_digit < NUM_MOTORS_PER_DIGIT;
			     motor_in_digit++) {
				const int motor_id = digit * NUM_MOTORS_PER_DIGIT + motor_in_digit;

				// Calculate wave position relative to this motor
				const int relative_position =
				    (digit * NUM_MOTORS_PER_DIGIT + motor_in_digit) - current_wave_position_;

				// Create sine wave pattern
				if (relative_position >= 0 && relative_position < 6) {
					const float angle_rad = (relative_position * M_PI) / 6.0f;
					const uint16_t target_angle =
					    static_cast<uint16_t>(WAVE_AMPLITUDE * sin(angle_rad));

					static constexpr uint32_t DEGREES_PER_ROTATION = 360;
					const uint16_t target_steps = static_cast<uint16_t>(
					    (static_cast<uint32_t>(target_angle) * NUM_STEPS_PER_ROT) /
					    DEGREES_PER_ROTATION);
					motor_move_to_absolute(motor_id, target_steps);
				}
			}
		}

		current_wave_position_++;
		if (current_wave_position_ >= NUM_MOTORS + 6) {
			current_wave_position_ = 0;
		}
	}
}

void WaveAnimation::stop()
{
	state_ = AnimationState::FINISHED;
	DBG_ANIM_LN("Wave animation finished");
}
