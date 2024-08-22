#include <Arduino.h>

#include "button/button.hpp"
#include "mode/mode.hpp"
#include "motor/motor_motion.h"
#include "motor/shift_register.h"
#include "time_manager.hpp"

void setup()
{
#ifdef DEBUG
	Serial.begin(115200);
	Serial.println("start");
#endif /* Debug */
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

	display_time();
#ifdef DEBUG
	Serial.println("Start!");
#endif /* Debug */
}

void loop()
{
#ifdef DEBUG
	const unsigned long time_us_0 = micros();
#endif /* DEBUG */

	loop_buttons();
	loop_mode();

	loop_motors();

#ifdef DEBUG
	const unsigned long time_us_1 = micros();
	Serial.print("Loop timing (us): ");
	Serial.println(time_us_1 - time_us_0);
#endif /* DEBUG */
}
