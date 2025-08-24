#include <Arduino.h>

#include "animation/animation_manager.hpp"
#include "button/button.hpp"
#include "calibration.hpp"
#include "cfg_time.hpp"
#include "motor/motor_motion.h"
#include "time_manager.hpp"

#ifdef DEBUG_MODE
#define DBG_MODE(...)    Serial.print(__VA_ARGS__)
#define DBG_MODE_LN(...) Serial.println(__VA_ARGS__)
#else
#define DBG_MODE(...)
#define DBG_MODE_LN(...)
#endif

enum clock_mode_t {
	MODE_CLOCK_DISPLAY,
	MODE_CLOCK_CONFIG,
	MODE_CALIB,
	MODE_SHUTDOWN,
};

static clock_mode_t _mode = MODE_CLOCK_DISPLAY;

#define MODE_TIMEOUT_MS 20000

static void _init_mode()
{
	button_reset();

	if (MODE_CALIB == _mode) {
		/* Reset calib state */
		calib_init();
		DBG_MODE_LN("MODE_CALIB");
	} else if (MODE_CLOCK_CONFIG == _mode) {
		/* Reset config state */
		DBG_MODE_LN("MODE_CLOCK_CONFIG");
	} else if (MODE_CLOCK_DISPLAY == _mode) {
		display_time();
		DBG_MODE_LN("MODE_CLOCK_DISPLAY");
	} else if (MODE_SHUTDOWN == _mode) {
		/* Set motors to neutral position */
		motors_goto_zero();
		DBG_MODE_LN("MODE_SHUTDOWN");
	}
}

static void _exit_mode()
{
	if (MODE_CALIB == _mode) {
		/* After the timeout, all motors should be not running anymore */
		motor_set_0_position();
	} else if (MODE_CLOCK_CONFIG == _mode) {
	} else if (MODE_CLOCK_DISPLAY == _mode) {
	} else if (MODE_SHUTDOWN == _mode) {
	}
}

static void _update_mode()
{
	button_t bt_mode = {};
	button_t bt_shutdown = {};
	const unsigned long time_ms = millis();
	const unsigned long last_press_ms = button_last_press();
	clock_mode_t new_mode = _mode;

	button_get_state(&bt_shutdown, BUTTON_ENCODER);
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
		_exit_mode();
		_mode = new_mode;
		_init_mode();
	}
}

void loop_mode()
{
	_update_mode();

	switch (_mode) {
	case MODE_CLOCK_DISPLAY:
		// Update animations only in display mode
		AnimationManager::getInstance().update();
		time_check();
		break;
	case MODE_CALIB:
		// Stop any running animation during calibration
		AnimationManager::getInstance().stopCurrentAnimation();
		loop_calib();
		break;
	case MODE_CLOCK_CONFIG:
		// Stop any running animation during configuration
		AnimationManager::getInstance().stopCurrentAnimation();
		loop_cfg_time();
		break;
	default:
		break;
	}
}
