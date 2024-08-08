#include <Arduino.h>

#include "animation.h"
#include "shift_register.h"
#include "time_manager.hpp"

void setup()
{
	Serial.begin(9600);
	Serial.println("start");
	shift_reg_init();

#if 0
	ctrl_test();
	uint8_t shift_bit_reg[] = {0b11001100, 0b10001010};
	ctrl_motors(shift_bit_reg, 16);

	set_time_clock();
#else
	test_anim();
#endif
}

void loop()
{
	process_anim();
}