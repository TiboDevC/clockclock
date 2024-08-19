#ifndef CLOCKCLOCK_ANIMATION_2_H
#define CLOCKCLOCK_ANIMATION_2_H

void set_clock_time(int h, int m);
void loop_motors();
void increment_needle_pos(const int motor_idx, int16_t increment);
void animation_init();
void motion_mode_set_calib();
void motion_mode_set_normal();

#endif /* CLOCKCLOCK_ANIMATION_2_H */
