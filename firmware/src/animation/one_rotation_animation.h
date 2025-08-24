#pragma once

#include "animation.hpp"

class OneRotationAnimation final : public Animation
{
public:
	explicit OneRotationAnimation(const uint32_t duration_ms = 60000)
	    : Animation(duration_ms)
	{
	}

	void start() override;
	void update(uint32_t current_time_ms) override;
	void stop() override;
};
