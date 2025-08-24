#include <stdint.h>

#include "button/button.hpp"
#include "time_manager.hpp"

enum cfg_time_select_t : uint8_t {
	CFG_TIME_MIN,
	CFG_TIME_HOUR,
};

static struct {
	cfg_time_select_t select;
	uint8_t increment;
} _ctx = {};

void loop_cfg_time()
{
	button_t bt_encoder = {};

	button_get_state(&bt_encoder, BUTTON_ENCODER);

	if (NO_PRESS != bt_encoder.press) {
		if (CFG_TIME_HOUR == _ctx.select) {
			_ctx.select = CFG_TIME_MIN;
		} else {
			_ctx.select = CFG_TIME_HOUR;
		}
	}

	const int16_t increment_count = button_get_encoder_count();
	if (0 != increment_count) {
		if (_ctx.increment) {
			if (CFG_TIME_MIN == _ctx.select) {
				rtc_increment_time_min(increment_count * 60);
			} else {
				rtc_increment_time_min(increment_count * 3600);
			}
			display_time();
		}
		/* Hack because one physical increment of the encoder represents 2 electronic increments */
		_ctx.increment++;
		_ctx.increment %= 2;
	}
}
