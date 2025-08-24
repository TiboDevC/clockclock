#pragma once
#include <cstdint>

#include "cfg.hpp"

static constexpr uint32_t DEGREES_PER_ROTATION = 360;

static constexpr uint16_t angleToSteps(const uint16_t target_angle) noexcept
{
	return static_cast<uint16_t>((static_cast<uint32_t>(target_angle) * NUM_STEPS_PER_ROT) /
	                             DEGREES_PER_ROTATION);
}

static constexpr uint32_t stepToAngle(const uint32_t target_step) noexcept
{
	return (target_step * DEGREES_PER_ROTATION) / NUM_STEPS_PER_ROT;
}
