/* encoder.c
 * Simple encoder interface using TIM encoder mode (TIM3 and TIM4)
 * Call Encoder_Init() after MX_TIMx_Init(), then call Encoder_Update(dt_ms)
 * from a periodic task (e.g., every 20-100 ms) to compute speed.
 */

#include "encoder.h"
#include "tim.h"
#include <math.h>

/* Internal state */
static int32_t acc_left = 0;
static int32_t acc_right = 0;
static int32_t last_cnt_left = 0;
static int32_t last_cnt_right = 0;
static float speed_rps_left = 0.0f;
static float speed_rps_right = 0.0f;

/* Helper: signed difference for 16-bit timer with wrap-around handling */
static int32_t diff16(uint16_t now, uint16_t last)
{
    int32_t d = (int32_t)now - (int32_t)last;
    if (d > 32767) d -= 65536;
    else if (d < -32768) d += 65536;
    return d;
}

void Encoder_Init(void)
{
    /* start encoder interface on TIM3 and TIM4 (channels configured in MX_TIMx_Init)
       adjust if your left/right mapping differs */
    HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);
    HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_ALL);

    last_cnt_left = (int32_t)__HAL_TIM_GET_COUNTER(&htim3);
    last_cnt_right = (int32_t)__HAL_TIM_GET_COUNTER(&htim4);

    acc_left = 0;
    acc_right = 0;
    speed_rps_left = 0.0f;
    speed_rps_right = 0.0f;
}

void Encoder_Update(uint32_t dt_ms)
{
    if (dt_ms == 0) return;

    uint16_t cnt_l = (uint16_t)__HAL_TIM_GET_COUNTER(&htim3);
    uint16_t cnt_r = (uint16_t)__HAL_TIM_GET_COUNTER(&htim4);

    int32_t d_l = diff16(cnt_l, (uint16_t)last_cnt_left);
    int32_t d_r = diff16(cnt_r, (uint16_t)last_cnt_right);

    /* accumulate raw counts (signed) */
    acc_left += d_l;
    acc_right += d_r;

    /* counts per second */
    float cps_l = d_l * (1000.0f / (float)dt_ms);
    float cps_r = d_r * (1000.0f / (float)dt_ms);

    /* convert to revolutions per second */
    speed_rps_left = cps_l / (float)ENCODER_PULSES_PER_REV;
    speed_rps_right = cps_r / (float)ENCODER_PULSES_PER_REV;

    last_cnt_left = cnt_l;
    last_cnt_right = cnt_r;
}

int32_t Encoder_GetCountLeft(void)
{
    return acc_left;
}

int32_t Encoder_GetCountRight(void)
{
    return acc_right;
}

void Encoder_ResetLeft(void)
{
    acc_left = 0;
    __HAL_TIM_SET_COUNTER(&htim3, 0);
    last_cnt_left = 0;
}

void Encoder_ResetRight(void)
{
    acc_right = 0;
    __HAL_TIM_SET_COUNTER(&htim4, 0);
    last_cnt_right = 0;
}

float Encoder_GetSpeedLeft_rps(void)
{
    return speed_rps_left;
}

float Encoder_GetSpeedRight_rps(void)
{
    return speed_rps_right;
}

float Encoder_GetSpeedLeft_rpm(void)
{
    return speed_rps_left * 60.0f;
}

float Encoder_GetSpeedRight_rpm(void)
{
    return speed_rps_right * 60.0f;
}

float Encoder_GetSpeedLeft_mps(void)
{
    /* v = rps * circumference */
    return speed_rps_left * (3.1415926f * ENCODER_WHEEL_DIAMETER_M);
}

float Encoder_GetSpeedRight_mps(void)
{
    return speed_rps_right * (3.1415926f * ENCODER_WHEEL_DIAMETER_M);
}
