#include <Arduino.h>

#include "button/button.hpp"
#include "calibration.hpp"
#include "motor/motor_motion.h"
#include "time_manager.hpp"

enum mode_t {
	MODE_CLOCK_DISPLAY,
	MODE_CLOCK_CONFIG,
	MODE_CALIB,
	MODE_SHUTDOWN,
};

static mode_t _mode = MODE_CLOCK_DISPLAY;

#define MODE_TIMEOUT_MS 10000

static void _init_mode()
{
	button_reset();
	motion_mode_set_normal();

	if (MODE_CALIB == _mode) {
		/* Reset calib state */
		motion_mode_set_calib();
	} else if (MODE_CLOCK_CONFIG == _mode) {
		/* Reset config state */
	} else if (MODE_CLOCK_DISPLAY == _mode) {
		display_time();
	} else if (MODE_SHUTDOWN == _mode) {
		/* Set motors to neutral position */
		motion_set_motor_neutral();
	}
}

static void _update_mode()
{
	struct button_t bt_mode = {};
	struct button_t bt_shutdown = {};
	const unsigned long time_ms = millis();
	const unsigned long last_press_ms = button_last_press();
	enum mode_t new_mode = _mode;

	button_get_state(&bt_shutdown, BUTTON_SHUTDOWN);
	button_get_state(&bt_mode, BUTTON_MODE);

	if (LONG_PRESS == bt_shutdown.press) {
		new_mode = MODE_SHUTDOWN;
	} else if (LONG_PRESS == bt_mode.press) {
		new_mode = MODE_CALIB;
	} else if (SHORT_PRESS == bt_mode.press) {
		new_mode = MODE_CLOCK_CONFIG;
	} else if (MODE_SHUTDOWN != _mode &&
	           (time_ms < last_press_ms || time_ms - last_press_ms > MODE_TIMEOUT_MS)) {
		new_mode = MODE_CLOCK_DISPLAY;
	}

	if (new_mode != _mode) {
		_mode = new_mode;
		_init_mode();
	}
}

void loop_mode()
{
	_update_mode();

	switch (_mode) {
	case MODE_CLOCK_DISPLAY:
		time_check();
		break;
	case MODE_CALIB:
		loop_calib();
		break;
	default:
		break;
	}
}
