#ifndef CLOCKCLOCK_ANIMATION_HPP
#define CLOCKCLOCK_ANIMATION_HPP

#include <cstdint>

enum class AnimationType : uint8_t { SYNC_ROTATION, WAVE_PATTERN };

enum class AnimationState : uint8_t { IDLE, RUNNING, FINISHED };

class Animation
{
public:
	explicit Animation(const uint32_t duration_ms)
	    : duration_ms_(duration_ms)
	{
	}

	virtual ~Animation() = default;

	// Pure virtual methods
	virtual void start() = 0;
	virtual void update(uint32_t current_time_ms) = 0;
	virtual void stop() = 0;
	virtual AnimationType getType() const = 0;

	// Common interface
	AnimationState getState() const
	{
		return state_;
	}

	uint32_t getDuration() const
	{
		return duration_ms_;
	}

	bool isFinished() const
	{
		return state_ == AnimationState::FINISHED;
	}

protected:
	AnimationState state_ = AnimationState::IDLE;
	uint32_t duration_ms_;
	uint32_t start_time_ms_ = 0;
};

#endif /* CLOCKCLOCK_ANIMATION_HPP */
