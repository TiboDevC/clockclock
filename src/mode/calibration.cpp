#include <stdint.h>

#include "button/button.hpp"
#include "cfg.hpp"
#include "motor/motor_motion.h"

static struct {
	uint8_t motor_idx;
} _ctx = {};

void calib_init()
{
	_ctx.motor_idx = 0;
	/* Reset rotary encoder count */
	button_get_encoder_count();
}

void loop_calib()
{
	struct button_t bt_encoder = {};

	button_get_state(&bt_encoder, BUTTON_ENCODER);

	if (NO_PRESS != bt_encoder.press) {
		_ctx.motor_idx++;
		_ctx.motor_idx %= NUM_MOTORS;
	}

	const int8_t increment_count = button_get_encoder_count();
	if (0 != increment_count) {
		increment_needle_pos(_ctx.motor_idx, increment_count);
	}
}
