#ifndef CLOCKCLOCK_WAVE_ANIMATION_HPP
#define CLOCKCLOCK_WAVE_ANIMATION_HPP

#include "animation.hpp"

class WaveAnimation final : public Animation
{
public:
	explicit WaveAnimation(uint32_t duration_ms = 30000)
	    : Animation(duration_ms)
	{
	}

	void start() override;
	void update(uint32_t current_time_ms) override;
	void stop() override;
	AnimationType getType() const override
	{
		return AnimationType::WAVE_PATTERN;
	}

private:
	enum class WavePhase {
		INITIALIZING, // Move all hands to 0°
		ROTATING      // Sequential rotations
	};

	static constexpr uint32_t DELAY_BETWEEN_MOTORS_MS = 500; // 500ms between each motor start

	WavePhase current_phase_ = WavePhase::INITIALIZING;
	int current_rotating_motor_ = 0; // Which motor is currently starting rotation
	uint32_t last_motor_start_time_ = 0;
};

#endif /* CLOCKCLOCK_WAVE_ANIMATION_HPP */
