#include "DS3231.h"
#include "animation/animation_manager.hpp"
#include "motor/motion.hpp"

#ifdef DEBUG_TIME_MGMT
#define DBG_TIME_MGMT(...)    Serial.print(__VA_ARGS__)
#define DBG_TIME_MGMT_LN(...) Serial.println(__VA_ARGS__)
#else
#define DBG_TIME_MGMT(...)
#define DBG_TIME_MGMT_LN(...)
#endif

/*
 * SDA/SCL pin defined in `variants/esp32c3/pins_arduino.h`
 * */

static constexpr uint32_t CHECK_TIME_DELAY_MS = 500;

// DST configuration
static bool dst_enabled = true;         // Enable DST by default
static int32_t winter_offset_hours = 1; // UTC+1 (CET) for winter
static int32_t summer_offset_hours = 2; // UTC+2 (CEST) for summer

// DST cache to avoid recalculating every time
static struct {
	int cached_year = -1;
	int cached_month = -1;
	int cached_day = -1;
	bool dst_active = false;
	int32_t current_offset_hours = 1;
} dst_cache;

// Helper function to get day of week (0=Sunday, 1=Monday, ..., 6=Saturday)
static int get_day_of_week(int year, int month, int day)
{
	// Zeller's congruence algorithm
	if (month < 3) {
		month += 12;
		year--;
	}

	const int k = year % 100;
	const int j = year / 100;

	const int h = (day + ((13 * (month + 1)) / 5) + k + (k / 4) + (j / 4) - 2 * j) % 7;

	// Convert to standard format (0=Sunday)
	return (h + 5) % 7;
}

// Find last Sunday of a month
static int get_last_sunday(int year, int month)
{
	// Start from the last day of the month and work backwards
	const int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	int last_day = days_in_month[month - 1];

	// Adjust for leap year
	if (month == 2 && ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))) {
		last_day = 29;
	}

	// Find the last Sunday
	for (int day = last_day; day >= 1; day--) {
		if (get_day_of_week(year, month, day) == 0) { // Sunday
			return day;
		}
	}
	return -1; // Should never happen
}

bool is_dst_active(int year, int month, int day)
{
	if (!dst_enabled) {
		return false;
	}

	// European DST rules:
	// DST starts: Last Sunday in March at 2:00 AM
	// DST ends: Last Sunday in October at 3:00 AM

	const int march_last_sunday = get_last_sunday(year, 3);
	const int october_last_sunday = get_last_sunday(year, 10);

	// Before March or after October
	if (month < 3 || month > 10) {
		return false;
	}

	// During April to September (definitely DST)
	if (month > 3 && month < 10) {
		return true;
	}

	// March: DST starts on last Sunday
	if (month == 3) {
		return day >= march_last_sunday;
	}

	// October: DST ends on last Sunday
	if (month == 10) {
		return day < october_last_sunday;
	}

	return false;
}

static void update_dst_cache_if_needed(int year, int month, int day)
{
	// Only recalculate if date changed
	if (dst_cache.cached_year == year && dst_cache.cached_month == month && dst_cache.cached_day == day) {
		return; // Cache is still valid
	}

	// Update cache
	dst_cache.cached_year = year;
	dst_cache.cached_month = month;
	dst_cache.cached_day = day;
	dst_cache.dst_active = dst_enabled ? is_dst_active(year, month, day) : false;
	dst_cache.current_offset_hours = dst_cache.dst_active ? summer_offset_hours : winter_offset_hours;

	DBG_TIME_MGMT("DST cache updated for ");
	DBG_TIME_MGMT(year);
	DBG_TIME_MGMT("/");
	DBG_TIME_MGMT(month);
	DBG_TIME_MGMT("/");
	DBG_TIME_MGMT(day);
	DBG_TIME_MGMT(" - DST: ");
	DBG_TIME_MGMT(dst_cache.dst_active ? "YES" : "NO");
	DBG_TIME_MGMT(", Offset: UTC+");
	DBG_TIME_MGMT_LN(dst_cache.current_offset_hours);
}

void get_current_time_with_dst(int &hour, int &minute)
{
	const DateTime now = RTClib::now();
	hour = now.hour();
	minute = now.minute();

	if (dst_enabled) {
		// Update DST cache only if date changed
		update_dst_cache_if_needed(now.year(), now.month(), now.day());

		// Apply cached timezone offset (RTC is assumed to be in UTC)
		hour += dst_cache.current_offset_hours;

		// Handle day overflow
		if (hour >= 24) {
			hour -= 24;
		} else if (hour < 0) {
			hour += 24;
		}
	}
}

void set_dst_timezone(bool enable_dst, int32_t winter_offset_hours_param, int32_t summer_offset_hours_param)
{
	dst_enabled = enable_dst;
	winter_offset_hours = winter_offset_hours_param;
	summer_offset_hours = summer_offset_hours_param;

	// Invalidate cache to force recalculation
	dst_cache.cached_year = -1;
	dst_cache.cached_month = -1;
	dst_cache.cached_day = -1;

	DBG_TIME_MGMT("DST configuration: ");
	DBG_TIME_MGMT(dst_enabled ? "ENABLED" : "DISABLED");
	DBG_TIME_MGMT(", Winter: UTC+");
	DBG_TIME_MGMT(winter_offset_hours);
	DBG_TIME_MGMT(", Summer: UTC+");
	DBG_TIME_MGMT_LN(summer_offset_hours);
}

void display_time(void)
{
	// Don't update time display during animations
	if (AnimationManager::getInstance().isAnimationRunning()) {
		return;
	}

	static DateTime old_time = {0};
	static bool old_dst_status = false;

	const DateTime now = RTClib::now();

	// Update DST cache if needed
	update_dst_cache_if_needed(now.year(), now.month(), now.day());

	// Update if time changed OR DST status changed
	if ((now.minute() != old_time.minute() || now.hour() != old_time.hour()) ||
	    (dst_cache.dst_active != old_dst_status)) {
		old_time = now;
		old_dst_status = dst_cache.dst_active;

		int display_hour, display_minute;
		get_current_time_with_dst(display_hour, display_minute);
		set_clock_time(display_hour, display_minute);
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

	// Always update time display (but display_time will check for animations)
	display_time();

	// Only check for new animations if none is running
	if (!AnimationManager::getInstance().isAnimationRunning()) {
		int current_hour, current_minute;
		get_current_time_with_dst(current_hour, current_minute);
		AnimationManager::getInstance().checkScheduledAnimations(current_hour, current_minute);
	}
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

void get_current_time(int &hour, int &minute)
{
	// Use DST-aware time by default
	get_current_time_with_dst(hour, minute);
}

void restore_time_display()
{
	int hour, minute;
	get_current_time_with_dst(hour, minute);
	set_clock_time(hour, minute);
}
