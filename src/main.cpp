#include <Arduino.h>

#include "animation.h"
#include "shift_register.h"
#include "time_manager.hpp"

void setup()
{
	Serial.begin(115200);
	Serial.println("start");
	shift_reg_init();
	rtc_init();

#if 0
	ctrl_test();
	uint8_t shift_bit_reg[] = {0b11001100, 0b10001010};
	ctrl_motors(shift_bit_reg, 16);

	set_time_clock();
	test_anim();
#else

#endif

	set_clock_time(12, 34);
}

void loop()
{
	step_motors();
	// time_check();
}