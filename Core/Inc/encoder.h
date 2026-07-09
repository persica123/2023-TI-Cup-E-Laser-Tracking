/* encoder.h
 * Motor encoder speed measurement for STM32F4
 * Place in Core/Inc and call Encoder_Update periodically (e.g., 50 ms)
 */

#ifndef __ENCODER_H__
#define __ENCODER_H__

#include "main.h"
#include "tim.h"
#include <stdint.h>

/* Configure these for your encoder/wheel */
#ifndef ENCODER_PULSES_PER_REV
#define ENCODER_PULSES_PER_REV  (20*4)  /* default: 20 CPR * 4 (quadrature) */
#endif

#ifndef ENCODER_WHEEL_DIAMETER_M
#define ENCODER_WHEEL_DIAMETER_M  0.065f /* wheel diameter in meters (example) */
#endif

/* Initialize encoder timers (starts HAL TIM encoder) */
void Encoder_Init(void);

/* Must be called periodically with elapsed milliseconds since last call */
void Encoder_Update(uint32_t dt_ms);

/* Cumulative counts since last reset */
int32_t Encoder_GetCountLeft(void);
int32_t Encoder_GetCountRight(void);
void Encoder_ResetLeft(void);
void Encoder_ResetRight(void);

/* Speeds */
float Encoder_GetSpeedLeft_rps(void);   /* revolutions per second */
float Encoder_GetSpeedRight_rps(void);
float Encoder_GetSpeedLeft_rpm(void);   /* revolutions per minute */
float Encoder_GetSpeedRight_rpm(void);
float Encoder_GetSpeedLeft_mps(void);   /* meters per second */
float Encoder_GetSpeedRight_mps(void);

#endif /* __ENCODER_H__ */
