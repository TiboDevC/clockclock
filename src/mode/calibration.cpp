#include <stdint.h>

#include <HardwareSerial.h>

#include "button/button.hpp"
#include "cfg.hpp"
#include "motor/motor_motion.h"

#define ENCODER_FACTOR 5

#ifdef DEBUG_CALIB
#define DBG_CALIB(...)    Serial.print(__VA_ARGS__)
#define DBG_CALIB_LN(...) Serial.println(__VA_ARGS__)
#else
#define DBG_CALIB(...)
#define DBG_CALIB_LN(...)
#endif

static struct {
	uint8_t motor_idx;
} _ctx = {};

void calib_init()
{
	_ctx.motor_idx = 0;
}

void loop_calib()
{
	struct button_t bt_encoder = {};

	button_get_state(&bt_encoder, BUTTON_ENCODER);

	if (NO_PRESS != bt_encoder.press) {
		_ctx.motor_idx++;
		_ctx.motor_idx %= NUM_MOTORS;
	}

	int16_t increment_count = button_get_encoder_count();
	if (0 != increment_count) {
		increment_count *= ENCODER_FACTOR;
		increment_count += motor_distance_to_go(_ctx.motor_idx);
		motor_move_to_relative(_ctx.motor_idx, increment_count);
		DBG_CALIB("Increment ");
		DBG_CALIB(_ctx.motor_idx);
		DBG_CALIB(" of ");
		DBG_CALIB(increment_count);
		DBG_CALIB_LN("");
	}
}
