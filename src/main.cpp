#include <Arduino.h>

#include "animation.h"
#include "button/button.hpp"
#include "mode/mode.hpp"
#include "shift_register.h"
#include "time_manager.hpp"

void setup()
{
	Serial.begin(9600);
	Serial.println("start");
	shift_reg_init();
	rtc_init();
	button_init();
	animation_init();

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

void loop()
{
	const unsigned long time_us_0 = micros();

	button_check();
	mode_check();

	step_motors();

	const unsigned long time_us_1 = micros();
	Serial.print("Loop timing (us): ");
	Serial.println(time_us_1 - time_us_0);
}
