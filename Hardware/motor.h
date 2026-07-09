#ifndef __MOTOR_H
#define __MOTOR_H

#include "main.h"


#define MOTOR_PWM_MAX 100  

// ???? TIM2 ?? (? CubeMX ??)
extern TIM_HandleTypeDef htim9;

/* ================= ??????? ================= */
// ???? (Motor A) - ?? AIN1 ? AIN2
#define LEFT_IN1_PORT  GPIOB
#define LEFT_IN1_PIN   GPIO_PIN_1
#define LEFT_IN2_PORT  GPIOC
#define LEFT_IN2_PIN   GPIO_PIN_5

// ???? (Motor B) - ?? BIN1 ? BIN2
#define RIGHT_IN1_PORT GPIOE
#define RIGHT_IN1_PIN  GPIO_PIN_10
#define RIGHT_IN2_PORT GPIOE
#define RIGHT_IN2_PIN  GPIO_PIN_8

/* ================= ????? ================= */
void Motor_Init(void);
void Motor_Set_Left(int16_t speed);
void Motor_Set_Right(int16_t speed);
void Motor_Stop(void);

#endif
