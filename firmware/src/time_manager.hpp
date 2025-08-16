#ifndef CLOCKCLOCK_TIME_MANAGER_HPP
#define CLOCKCLOCK_TIME_MANAGER_HPP

#include <stdint.h>

void display_time(void);
void time_check(void);
void rtc_print_time(void);
void rtc_init(void);
void rtc_increment_time_min(int16_t min);
void get_current_time(int &hour, int &minute);
void restore_time_display();

#endif /* CLOCKCLOCK_TIME_MANAGER_HPP */
