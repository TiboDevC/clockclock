#include <stddef.h>

#include "DS3231.h"

#include "animation.h"

#define NUM_ELEM_STRUCT(elem) (sizeof(elem) / sizeof(elem[0]))

static const struct anim_pos_t _pos_num_0[] = {
    {.position_degree = 180, .motor_id = 0},
    {.position_degree = 90, .motor_id = 1},

    {.position_degree = 0, .motor_id = 2},
    {.position_degree = 180, .motor_id = 3},

    {.position_degree = 0, .motor_id = 4},
    {.position_degree = 90, .motor_id = 5},

    {.position_degree = 270, .motor_id = 6},
    {.position_degree = 0, .motor_id = 7},

    {.position_degree = 0, .motor_id = 8},
    {.position_degree = 180, .motor_id = 9},

    {.position_degree = 270, .motor_id = 10},
    {.position_degree = 180, .motor_id = 11},
};

static const struct anim_sequence_t _seq_num_0[] = {
    {.num_motors = NUM_ELEM_STRUCT(_pos_num_0),
     .param =
         {
	     .anim_type = ANIM_SYNC,
	     .anim_angle_type = ANGLE_ABSOLUTE,
	     .time_ms = 20,
	 },
     .position = _pos_num_0},
};

static const struct anim_pos_t _pos_num_1[] = {
    {.position_degree = 180, .motor_id = 0},
    {.position_degree = 90, .motor_id = 1},

    {.position_degree = 0, .motor_id = 2},
    {.position_degree = 180, .motor_id = 3},

    {.position_degree = 0, .motor_id = 4},
    {.position_degree = 90, .motor_id = 5},

    {.position_degree = 270, .motor_id = 6},
    {.position_degree = 0, .motor_id = 7},

    {.position_degree = 0, .motor_id = 8},
    {.position_degree = 180, .motor_id = 9},

    {.position_degree = 270, .motor_id = 10},
    {.position_degree = 180, .motor_id = 11},
};

static const struct anim_sequence_t _seq_num_1[] = {
    {.num_motors = NUM_ELEM_STRUCT(_pos_num_1),
     .param =
         {
	     .anim_type = ANIM_SYNC,
	     .anim_angle_type = ANGLE_ABSOLUTE,
	     .time_ms = 20,
	 },
     .position = _pos_num_1},
};

static const struct anim_sequence_t *_seq_numbers[] = {
    _seq_num_0, _seq_num_1, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

#define OFFSET_COLUMN(column) (3 * 2 * column)
#define COLUMN_NUM_0          (0 * 2)
#define COLUMN_NUM_1          (1 * 2)
#define COLUMN_NUM_2          (2 * 2)
#define COLUMN_NUM_3          (3 * 2)

static void _time_set_clock(const DateTime *now)
{
	const uint8_t hour = now->hour();
	const uint8_t minute = now->minute();

	Serial.print("time: ");
	Serial.print(hour);
	Serial.print(":");
	Serial.println(minute);

	seq_set(_seq_numbers[hour / 10], OFFSET_COLUMN(COLUMN_NUM_0));
	seq_set(_seq_numbers[hour % 10], OFFSET_COLUMN(COLUMN_NUM_1));
	seq_set(_seq_numbers[minute / 10], OFFSET_COLUMN(COLUMN_NUM_2));
	seq_set(_seq_numbers[minute % 10], OFFSET_COLUMN(COLUMN_NUM_3));
}

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
	if (is_anim_done() && (now.minute() != old_time.minute() || now.hour() != old_time.hour())) {
		_time_set_clock(&now);
		old_time = now;
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
	constexpr time_t tstmp{1702383132UL}; // Tue Dec 12 2023 11:12:12

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
