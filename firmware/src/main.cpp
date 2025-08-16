#include <Arduino.h>

#include "animation/animation_manager.hpp"
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

	// Initialize animation system
	AnimationManager::getInstance().init();

#if 0
	ctrl_test();
#endif

#if 0
	unsigned long last_time_ms = 0;

	int motor_id = 0;
	while (true) {
		const unsigned long time_ms = millis();

		if (time_ms > last_time_ms && time_ms - last_time_ms >= 1000) {
			last_time_ms = time_ms;
			motor_test(motor_id);
			Serial.print("Loop ");
			Serial.println(motor_id);
			motor_id++;
			if (motor_id >= NUM_MOTORS) {
				motor_id = 0;
			}
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
