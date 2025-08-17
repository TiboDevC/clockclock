#ifndef CLOCKCLOCK_TIME_MANAGER_HPP
#define CLOCKCLOCK_TIME_MANAGER_HPP

#include <stdint.h>

void display_time();
void time_check();
void rtc_print_time();
void rtc_init();
void rtc_increment_time_min(int16_t min);
void get_current_time(int &hour, int &minute);
void restore_time_display();

// DST (Daylight Saving Time) functions
bool is_dst_active(int year, int month, int day);
void get_current_time_with_dst(int &hour, int &minute);
void set_dst_timezone(bool enable_dst, int32_t winter_offset_hours = 1, int32_t summer_offset_hours = 2);

#endif /* CLOCKCLOCK_TIME_MANAGER_HPP */
