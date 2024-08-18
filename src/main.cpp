#include <Arduino.h>

#include "animation.h"
#include "button/button.hpp"
#include "shift_register.h"
#include "time_manager.hpp"

void setup()
{
	Serial.begin(9600);
	Serial.println("start");
	shift_reg_init();
	rtc_init();
	button_init();

#if 0
	ctrl_test();
	uint8_t shift_bit_reg[] = {0b11001100, 0b10001010};
	ctrl_motors(shift_bit_reg, 16);

	set_time_clock();
	test_anim();
#else

#endif

	set_clock_time(12, 34);
	Serial.println("Start!");
}

enum mode_t {
	MODE_CLOCK_DISPLAY,
	MODE_CLOCK_CONFIG,
	MODE_CALIB,
};

static mode_t _mode = MODE_CLOCK_DISPLAY;

#define MODE_TIMEOUT_MS 10000

static void _init_mode()
{
	button_reset();

	if (MODE_CALIB == _mode) {
		/* Reset calib state */
	} else if (MODE_CLOCK_CONFIG == _mode) {
		/* Reset config state */
	} else if (MODE_CLOCK_DISPLAY == _mode) {
		display_time();
	}
}

static void _update_mode()
{
	struct button_t button = {};
	const unsigned long time_ms = millis();
	const unsigned long last_press_ms = button_last_press();
	enum mode_t new_mode = _mode;

	button_get_state(&button, BUTTON_MODE);

	if (LONG_PRESS == button.press) {
		new_mode = MODE_CALIB;
	} else if (SHORT_PRESS == button.press) {
		new_mode = MODE_CLOCK_CONFIG;
	} else if (time_ms < last_press_ms || time_ms - last_press_ms > MODE_TIMEOUT_MS) {
		new_mode = MODE_CLOCK_DISPLAY;
	}

	if (new_mode != _mode) {
		_mode = new_mode;
		_init_mode();
	}
}

void loop()
{
	const unsigned long time_us_0 = micros();

	button_check();

	_update_mode();

	switch (_mode) {
	case MODE_CLOCK_DISPLAY:
		time_check();
		break;
	case MODE_CLOCK_CONFIG:
		break;
	case MODE_CALIB:
		break;
	}

	step_motors();

	const unsigned long time_us_1 = micros();
	Serial.print("Loop timing (us): ");
	Serial.println(time_us_1 - time_us_0);
}
