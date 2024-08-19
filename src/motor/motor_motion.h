#ifndef CLOCKCLOCK_ANIMATION_2_H
#define CLOCKCLOCK_ANIMATION_2_H

void set_clock_time(int h, int m);
void step_motors();
void increment_needle_pos(const int motor_idx, int16_t increment);
void animation_init();

#endif /* CLOCKCLOCK_ANIMATION_2_H */
