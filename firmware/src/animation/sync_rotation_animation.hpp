#ifndef CLOCKCLOCK_SYNC_ROTATION_ANIMATION_HPP
#define CLOCKCLOCK_SYNC_ROTATION_ANIMATION_HPP

#include "animation.hpp"

class SyncRotationAnimation final : public Animation
{
public:
	explicit SyncRotationAnimation(const uint32_t duration_ms = 60000)
	    : Animation(duration_ms)
	{
	}

	void start() override;
	void update(uint32_t current_time_ms) override;
	void stop() override;

private:
	static constexpr uint16_t TARGET_ANGLE = 180; // 0h30 position
	bool rotation_started_ = false;

	static constexpr float MAX_MOTOR_SPEED = 500;
};

#endif /* CLOCKCLOCK_SYNC_ROTATION_ANIMATION_HPP */
