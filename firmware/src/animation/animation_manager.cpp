#include <esp32-hal.h>

#include "../time_manager.hpp"
#include "animation_manager.hpp"
#include "sync_rotation_animation.hpp"
#include "wave_animation.hpp"

#ifdef DEBUG_ANIMATION
#define DBG_ANIM_MGR(...)    Serial.print(__VA_ARGS__)
#define DBG_ANIM_MGR_LN(...) Serial.println(__VA_ARGS__)
#else
#define DBG_ANIM_MGR(...)
#define DBG_ANIM_MGR_LN(...)
#endif

AnimationManager &AnimationManager::getInstance()
{
	static AnimationManager instance;
	return instance;
}

void AnimationManager::init()
{
	last_update_time_ = millis();
	DBG_ANIM_MGR_LN("Animation Manager initialized");
}

void AnimationManager::update()
{
	const uint32_t current_time = millis();

	if (current_time - last_update_time_ < UPDATE_INTERVAL_MS) {
		return;
	}

	last_update_time_ = current_time;

	if (current_animation_ && current_animation_->getState() == AnimationState::RUNNING) {
		current_animation_->update(current_time);

		if (current_animation_->isFinished()) {
			onAnimationComplete();
		}
	}
}

void AnimationManager::startAnimation(AnimationType type)
{
	if (current_animation_ && current_animation_->getState() == AnimationState::RUNNING) {
		DBG_ANIM_MGR_LN("Stopping current animation to start new one");
		current_animation_->stop();
	}

	current_animation_ = createAnimation(type);
	if (current_animation_) {
		DBG_ANIM_MGR("Starting animation type: ");
		DBG_ANIM_MGR_LN(static_cast<int>(type));
		current_animation_->start();
	}
}

void AnimationManager::startAnimation(std::unique_ptr<Animation> animation)
{
	if (current_animation_ || !animation) {
		return;
	}

	current_animation_ = std::move(animation);
	current_animation_->start();

	DBG_ANIM_MGR_LN("Animation started");
}

void AnimationManager::stopAnimation()
{
	if (current_animation_) {
		DBG_ANIM_MGR_LN("Stopping animation and restoring time display");
		current_animation_->stop();
		current_animation_.reset();

		// Restore time display from RTC
		restore_time_display();
	}
}

void AnimationManager::stopCurrentAnimation()
{
	if (current_animation_) {
		DBG_ANIM_MGR_LN("Force stopping current animation");
		current_animation_->stop();
		onAnimationComplete();
	}
}

bool AnimationManager::isAnimationRunning() const
{
	return current_animation_ && current_animation_->getState() == AnimationState::RUNNING;
}

void AnimationManager::checkScheduledAnimations(int current_hour, int current_minute)
{
	// Skip if animation is already running
	if (isAnimationRunning()) {
		return;
	}

	// Check for hour change (special celebration) - this takes priority over quarter-hour animations
	if (last_hour_check_ != -1 && last_hour_check_ != current_hour && current_minute == 0) {
		DBG_ANIM_MGR_LN("Hour change detected - starting celebration");
		startAnimation(AnimationType::SYNC_ROTATION);
		last_hour_check_ = current_hour;
		last_minute_check_ = current_minute;
		return; // Exit early to prevent quarter-hour animation at minute 0
	}

	// Check for quarter-hour animations (15, 30, 45 minutes only - excluding 0 since hour change handles
	// it)
	if (last_minute_check_ != current_minute) {
		static constexpr std::array QUARTER_MINUTES = {15, 30, 45};

		for (const auto &quarter_minute : QUARTER_MINUTES) {
			if (current_minute == quarter_minute) {
				// Alternate between different animation types
				AnimationType anim_type;
				switch (current_minute) {
				case 15:
					anim_type = AnimationType::WAVE_PATTERN;
					break;
				case 30:
					anim_type = AnimationType::SYNC_ROTATION;
					break;
				case 45:
					anim_type = AnimationType::WAVE_PATTERN;
					break;
				default:
					anim_type = AnimationType::SYNC_ROTATION;
					break;
				}

				DBG_ANIM_MGR("Quarter hour animation at minute: ");
				DBG_ANIM_MGR_LN(current_minute);
				startAnimation(anim_type);
				break;
			}
		}

		last_minute_check_ = current_minute;
	}

	// Update hour tracking
	last_hour_check_ = current_hour;
}

std::unique_ptr<Animation> AnimationManager::createAnimation(const AnimationType type)
{
	switch (type) {
	case AnimationType::SYNC_ROTATION:
		return std::make_unique<SyncRotationAnimation>();
	case AnimationType::WAVE_PATTERN:
		return std::make_unique<WaveAnimation>();
	default:
		return std::make_unique<WaveAnimation>();
	}
}

void AnimationManager::onAnimationComplete()
{
	DBG_ANIM_MGR_LN("Animation completed - restoring time display");
	current_animation_.reset();

	// Restore time display immediately
	restore_time_display();
}
