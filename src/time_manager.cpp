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

void set_time_clock(void)
{
	const DateTime now = RTClib::now();
	const uint8_t hour = now.hour();
	const uint8_t minute = now.minute();

	Serial.print("time: ");
	Serial.print(hour);
	Serial.print(":");
	Serial.println(minute);

	seq_set(_seq_numbers[hour / 10], OFFSET_COLUMN(COLUMN_NUM_0));
	seq_set(_seq_numbers[hour % 10], OFFSET_COLUMN(COLUMN_NUM_1));
	seq_set(_seq_numbers[minute / 10], OFFSET_COLUMN(COLUMN_NUM_2));
	seq_set(_seq_numbers[minute % 10], OFFSET_COLUMN(COLUMN_NUM_3));
}