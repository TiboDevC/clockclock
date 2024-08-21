#include "DS3231.h"
#include "motor/motor_motion.h"

#ifdef DEBUG_TIME_MGMT
#define DBG_TIME_MGMT(...)    Serial.print(__VA_ARGS__)
#define DBG_TIME_MGMT_LN(...) Serial.println(__VA_ARGS__)
#else
#define DBG_TIME_MGMT(...)
#define DBG_TIME_MGMT_LN(...)
#endif

#define CHECK_TIME_DELAY_MS 500

void display_time(void)
{
	static DateTime old_time = {0};

	DateTime now = RTClib::now();
	if ((now.minute() != old_time.minute() || now.hour() != old_time.hour())) {
		old_time = now;
		set_clock_time(now.hour(), now.minute());
	}
}

void time_check(void)
{
	static unsigned long last_time_ms = 0;

	const unsigned long time_ms = millis();

	if (time_ms > last_time_ms && time_ms - last_time_ms < CHECK_TIME_DELAY_MS) {
		return;
	}
	last_time_ms = time_ms;

	display_time();
}

void rtc_print_time(void)
{
#ifdef DEBUG_TIME_MGMT
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
#endif /* DEBUG_TIME_MGMT */
}

void rtc_init(void)
{
	constexpr time_t tstmp{1702383132UL}; // Tue Dec 12 2023 12:12:12

	DS3231 Clock;
	Wire.begin();

	DBG_TIME_MGMT_LN("Init DS3231");
	delay(500);

	if (Clock.getYear() < 23) {
		DBG_TIME_MGMT_LN("RTC lost time, reconfigure it");
		Clock.setEpoch(tstmp, false);
		// set to 24h
		Clock.setClockMode(false);
	}

	rtc_print_time();
}

void rtc_increment_time_min(int16_t min)
{
	DS3231 Clock;
	const DateTime now = RTClib::now();

	const time_t new_time = now.unixtime() + min;
	Clock.setEpoch(new_time, false);
	DBG_TIME_MGMT("Set time: ");
	DBG_TIME_MGMT_LN(new_time);
}
