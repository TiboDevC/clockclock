#ifndef SOFT_CFG_HPP
#define SOFT_CFG_HPP

#define NUM_MOTORS 48
/* 1 shift register for 4 motors */
#define NUM_MOTORS_PER_SHIFT_REG 4
#define SHIFT_REG_SIZE           (NUM_MOTORS / NUM_MOTORS_PER_SHIFT_REG)
#define NUM_STEPS_PER_ROT        (360 * 12) /* 12 steps per degree */

/* Clock display configuration */
#define NUM_MOTORS_PER_DIGIT 12
#define NUM_DIGITS           4

#endif /* SOFT_CFG_HPP */
