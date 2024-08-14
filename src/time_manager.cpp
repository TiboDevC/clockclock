#include "DS3231.h"
#include "animation.h"

#define CHECK_TIME_DELAY_MS 500
void time_check(void)
{
	static DateTime old_time = {0};
	static unsigned long last_time_ms = 0;

	const unsigned long time_ms = millis();

	if (time_ms > last_time_ms && time_ms - last_time_ms < CHECK_TIME_DELAY_MS) {
		return;
	}
	last_time_ms = time_ms;

	DateTime now = RTClib::now();
	if ((now.minute() != old_time.minute() || now.hour() != old_time.hour())) {
		old_time = now;
		set_clock_time(now.hour(), now.minute());
	}
}

void rtc_print_time(void)
{
	const DateTime now = RTClib::now();

	Serial.print(now.year(), DEC);
	Serial.print('/');
	Serial.print(now.month(), DEC);
	Serial.print('/');
	Serial.print(now.day(), DEC);
	Serial.print(' ');
	Serial.print(now.hour(), DEC);
	Serial.print(':');
	Serial.print(now.minute(), DEC);
	Serial.print(':');
	Serial.print(now.second(), DEC);
	Serial.println();
}

void rtc_init(void)
{
	constexpr time_t tstmp{1702383132UL}; // Tue Dec 12 2023 12:12:12

	DS3231 Clock;
	Wire.begin();

	Serial.println("Init DS3231");
	delay(500);

	if (Clock.getYear() < 23) {
		Serial.println("RTC lost time, reconfigure it");
		Clock.setEpoch(tstmp, false);
		// set to 24h
		Clock.setClockMode(false);
	}

	rtc_print_time();
}
