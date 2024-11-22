#ifndef CLOCKCLOCK_TIME_MANAGER_HPP
#define CLOCKCLOCK_TIME_MANAGER_HPP

void rtc_init(void);
void display_time(void);
void time_check(void);
void rtc_increment_time_min(int16_t min);

#endif /* CLOCKCLOCK_TIME_MANAGER_HPP */
