#pragma once

#include "animation.hpp"

class BracketAnimation final : public Animation
{
public:
	explicit BracketAnimation(const uint32_t duration_ms = 60000)
	    : Animation(duration_ms)
	{
	}

	void start() override;
	void update(uint32_t current_time_ms) override;
	void stop() override;

private:
	bool rotation_started_ = false;
};
