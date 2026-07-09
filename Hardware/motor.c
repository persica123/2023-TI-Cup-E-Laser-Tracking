#include "motor.h"
#include <stdlib.h> // ?? abs() ??

/**
 * @brief ?? TIM2 ? PWM ??
 */
void Motor_Init(void) {
    HAL_TIM_PWM_Start(&htim9, TIM_CHANNEL_1); // PWMA
    HAL_TIM_PWM_Start(&htim9, TIM_CHANNEL_2); // PWMB
}

/**
 * @brief ??????? (-100 ? 100)
 */
void Motor_Set_Left(int16_t speed) {
    if (speed > MOTOR_PWM_MAX)  speed = MOTOR_PWM_MAX;
    if (speed < -MOTOR_PWM_MAX) speed = -MOTOR_PWM_MAX;

    if (speed >= 0) {
        HAL_GPIO_WritePin(LEFT_IN1_PORT, LEFT_IN1_PIN, GPIO_PIN_SET);
        HAL_GPIO_WritePin(LEFT_IN2_PORT, LEFT_IN2_PIN, GPIO_PIN_RESET);
    } else {
        HAL_GPIO_WritePin(LEFT_IN1_PORT, LEFT_IN1_PIN, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(LEFT_IN2_PORT, LEFT_IN2_PIN, GPIO_PIN_SET);
    }
    __HAL_TIM_SET_COMPARE(&htim9, TIM_CHANNEL_1, abs(speed));
}

/**
 * @brief ??????? (-100 ? 100)
 */
void Motor_Set_Right(int16_t speed) {
    if (speed > MOTOR_PWM_MAX)  speed = MOTOR_PWM_MAX;
    if (speed < -MOTOR_PWM_MAX) speed = -MOTOR_PWM_MAX;

    if (speed >= 0) {
        HAL_GPIO_WritePin(RIGHT_IN1_PORT, RIGHT_IN1_PIN, GPIO_PIN_SET);
        HAL_GPIO_WritePin(RIGHT_IN2_PORT, RIGHT_IN2_PIN, GPIO_PIN_RESET);
    } else {
        HAL_GPIO_WritePin(RIGHT_IN1_PORT, RIGHT_IN1_PIN, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(RIGHT_IN2_PORT, RIGHT_IN2_PIN, GPIO_PIN_SET);
    }
    __HAL_TIM_SET_COMPARE(&htim9, TIM_CHANNEL_2, abs(speed));
}

/**
 * @brief ???
 */
void Motor_Stop(void) {
    HAL_GPIO_WritePin(LEFT_IN1_PORT, LEFT_IN1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LEFT_IN2_PORT, LEFT_IN2_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(RIGHT_IN1_PORT, RIGHT_IN1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(RIGHT_IN2_PORT, RIGHT_IN2_PIN, GPIO_PIN_RESET);
    
    __HAL_TIM_SET_COMPARE(&htim9, TIM_CHANNEL_1, 0);
    __HAL_TIM_SET_COMPARE(&htim9, TIM_CHANNEL_2, 0);
}
