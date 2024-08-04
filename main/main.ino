#if 0

#define MOTOR_IN1 8
#define MOTOR_IN2 9
#define MOTOR_IN3 10
#define MOTOR_IN4 11

void setup() {
  pinMode(MOTOR_IN1, OUTPUT);
  pinMode(MOTOR_IN2, OUTPUT);
  pinMode(MOTOR_IN3, OUTPUT);
  pinMode(MOTOR_IN4, OUTPUT);

  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, LOW);
  digitalWrite(MOTOR_IN3, LOW);
  digitalWrite(MOTOR_IN4, LOW);
}

const int steps[4][4] =
    {
        {1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1},
};

static int step = 0;

void loop() {
  digitalWrite(MOTOR_IN1, steps[step][0]);
  digitalWrite(MOTOR_IN2, steps[step][1]);
  digitalWrite(MOTOR_IN3, steps[step][2]);
  digitalWrite(MOTOR_IN4, steps[step][3]);
  delay(2);
  step++;
  step %= 4;
}

#else

#include "shift_register.h"

void setup() {
#if 0
  const struct anim_pos_t anim[2] = {{.motor_id = 0,
                                      .anim_type = ANIM_SYNC,
                                      .anim_angle_type = ANGLE_RELATIVE,
                                      .position_degree = 90 + 360,
                                      .time_ms = 20},
                                     {.motor_id = 1,
                                      .anim_type = ANIM_ASYNC,
                                      .anim_angle_type = ANGLE_ABSOLUTE,
                                      .position_degree = 360 + 180,
                                      .time_ms = 20}};

  send_anim(anim, 2);
  print_motor_state();
  printf("Is anim done: %d\n", is_anim_done());
  printf("Is anim ready: %d\n", is_anim_ready());
  process_anim();
#endif

  Serial.begin(9600);
  Serial.println("start");
  shift_reg_init();
  //ctrl_test();
  uint8_t shift_bit_reg[] = {0b11001100, 0b10001010};
  ctrl_motors(shift_bit_reg, 16);
}

void loop() {
  Serial.println("top");
  delay(1000);
}

#endif
