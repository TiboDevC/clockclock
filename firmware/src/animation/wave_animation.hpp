#ifndef CLOCKCLOCK_WAVE_ANIMATION_HPP
#define CLOCKCLOCK_WAVE_ANIMATION_HPP

#include "animation.hpp"

class WaveAnimation final : public Animation
{
public:
	explicit WaveAnimation(uint32_t duration_ms = 8000)
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
	static constexpr uint32_t WAVE_INTERVAL_MS = 200;
	static constexpr uint16_t WAVE_AMPLITUDE = 90;          // 90 degrees
	static constexpr uint32_t RETURN_PHASE_DURATION = 2000; // 2 seconds for artistic return
	uint32_t last_wave_time_ = 0;
	int current_wave_position_ = 0;
	bool in_return_phase_ = false;
	uint32_t return_phase_start_time_ = 0;
};

#endif /* CLOCKCLOCK_WAVE_ANIMATION_HPP */
