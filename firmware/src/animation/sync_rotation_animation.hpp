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
	static constexpr float MAX_MOTOR_SPEED = 400;

	enum class AnimationPhase {
		ALIGNING,
		ROTATING,
		WAIT_ENDING,
	};
	AnimationPhase current_phase_ = AnimationPhase::ALIGNING;
	uint32_t last_motor_start_time_ = 0;
	int current_rotating_motor_ = 0;
};

#endif /* CLOCKCLOCK_SYNC_ROTATION_ANIMATION_HPP */
