enum anim_type_t {
  ANIM_SYNC,
  ANIM_ASYNC,
}

enum anim_angle_type_t {
  ANGLE_ABSOLUTE,
  ANGLE_RELATIVE,
}

struct anim_pos_t {
  uint8_t motor_id;
  enum anim_type_t anim_type;
  enum anim_angle_type_t anim_angle_type;
  int16_t position_degree;
  uint16_t time_ms;
}
/* motor_controller.c */

#define NUM_MOTORS (48)
#define NUM_STEPS_PER_ROT (4096)
#define MAX_MOTOR_STEP (32767)
#define MAX_MOTOR_ANGLE ((MAX_MOTOR_STEP / NUM_STEPS_PER_ROT) * 360)

#define ANGLE_TO_STEPS(target_angle) ((target_angle * NUM_STEPS_PER_ROT) / 360)

struct motor_status_s {
  uint8_t step_pos : 2;
  uint8_t anim_type : 1;
}

struct {
  uint16_t current_step;
  int16_t num_step_remaining;
  uint16_t delay_step_us;
  struct motor_status_s motor_status;
} _motors[NUM_MOTORS] = {0};

static int16_t _process_num_steps(uint16_t cur_step, int16_t target_angle,
                                  enum anim_angle_type_t anim_angle_type) {
  int16_t num_step_remaining = 0;
  if (ANGLE_ABSOLUTE == anim_angle_type) {
    /* Correct the angle if necessary */
    target_angle %= 360;
    if (target_angle < 0) {
      target_angle *= -1;
    }

    const uint16_t target_step = ANGLE_TO_STEPS(target_angle);
    /* Always move in the same direction */
    if (target_step > cur_step) {
      num_step_remaining = target_step - cur_step;
    } else {
      num_step_remaining = target_step + 360 - cur_step;
    }
  } else if (ANGLE_RELATIVE == anim_angle_type) {
  	/* Make sure we do not exceed the  max angle */
    if (target_angle > MAX_MOTOR_ANGLE) {
      target_angle = MAX_MOTOR_ANGLE;
    } else if (-MAX_MOTOR_ANGLE < target_angle) {
      target_angle = -MAX_MOTOR_ANGLE;
    }
    num_step_remaining += ANGLE_TO_STEPS(target_angle);
  }
}

int send_anim(const struct anim_pos_t* anim_pos, uint8_t num_anim) {
  /* Basic config check */
  if (NULL == anim_pos) {
    return -1;
  }
  for (int anim_id = 0; anim_id < num_anim; anim_id++) {
    if (anim_pos[anim_id].motor_id >= NUM_MOTORS) {
      /* This motor does not exit, error in the config */
      return -1;
    }
  }

  /* Check if animation is in blocking mode and not finished */
  for (int motor_id = 0; motor_id < NUM_MOTORS; motor_id++) {
    if (ANIM_SYNC == _motors[motor_id].motor_status.anim_type &&
        0 != _motors[motor_id].motor_status.num_step_remaining) {
      /* At least one animation is not done */
      return -1;
    }
  }

  /* All animations are done, apply the new one */
  for (int anim_id = 0; anim_id < num_anim; anim_id++) {
    const uint8_t motor_id = anim_pos[anim_id].motor_id;
    _motors[motor_id].motor_status.anim_type = anim_pos[anim_id].anim_type;
    _motors[motor_id].target_angle = anim_pos[anim_id].position_degree;
    _motors[motor_id].motor_status.anim_done = 0;
    /* TODO process delay_step_us */
  }
}
