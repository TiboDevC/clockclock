#ifndef MOTOR_ANIMATION_H
#define MOTOR_ANIMATION_H

#include "AccelStepper.h"
void motor_init();
void motor_loop();
void motors_goto_zero();
void motor_set_0_position();
void motor_move_to_relative(const int motor_idx, int16_t increment);
void motor_move_to_absolute(const int motor_idx, int16_t increment);
long motor_get_position(const int motor_idx);
long motor_distance_to_go(const int motor_idx);
void motor_test();

class Motor : public AccelStepper
{
public:
	Motor();

	void step4(long step) final;

	void step8(long step) final;

	void setMotorId(uint8_t motor_id);

	void enableOutputs() final;

	void disableOutputs() final;

private:
	uint8_t _motor_id{};
};

#endif /* MOTOR_ANIMATION_H */
