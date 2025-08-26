#ifndef CLOCKCLOCK_ANIMATION_MANAGER_HPP
#define CLOCKCLOCK_ANIMATION_MANAGER_HPP

#include <cstdint>
#include <memory>

#include "animation.hpp"
#include "cfg.hpp"

class AnimationManager
{
public:
	static AnimationManager &getInstance();

	void init();
	void update();

	// Animation control
	void startAnimation(AnimationType type);
	void startAnimation(std::unique_ptr<Animation> animation);
	void stopAnimation();
	void stopCurrentAnimation();
	bool isAnimationRunning() const;

	// Schedule management
	void checkScheduledAnimations(int current_hour, int current_minute);

private:
	AnimationManager() = default;
	~AnimationManager() = default;
	AnimationManager(const AnimationManager &) = delete;
	AnimationManager &operator=(const AnimationManager &) = delete;

	void saveCurrentSpeeds();
	void restoreSavedSpeeds() const;
	void saveCurrentAcceleration();
	void restoreSavedAcceleration() const;

	static std::unique_ptr<Animation> createAnimation(AnimationType type);
	void onAnimationComplete();

	// Animation state
	std::unique_ptr<Animation> current_animation_;

	// Schedule tracking
	int last_minute_check_ = -1;
	int last_hour_check_ = -1;

	// Timing
	static constexpr uint32_t UPDATE_INTERVAL_MS = 50;
	uint32_t last_update_time_ = 0;

	std::array<float, NUM_MOTORS> motor_speeds_{};
	std::array<float, NUM_MOTORS> motor_accelerations_{};
};

#endif /* CLOCKCLOCK_ANIMATION_MANAGER_HPP */
