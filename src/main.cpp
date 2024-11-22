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
	motor_init();

#if 0
	ctrl_test();
#endif

#if 0
	unsigned long last_time_ms = 0;

	while (1) {
		const unsigned long time_ms = millis();

		if (time_ms > last_time_ms && time_ms - last_time_ms >= 10000) {
			last_time_ms = time_ms;
			motor_test();
			Serial.print("Loop ");
			Serial.println(time_ms);
		}

		motor_loop();
	}
#endif
	display_time();
#ifdef DEBUG
	Serial.println("Start!");
#endif /* Debug */
}

void loop()
{
#if 0
	const unsigned long time_us_0 = micros();
#endif /* 0 */

	loop_buttons();
	loop_mode();

	motor_loop();

#if 0
	const unsigned long time_us_1 = micros();
	Serial.print("Loop timing (us): ");
	Serial.println(time_us_1 - time_us_0);
#endif /* 0 */
}
