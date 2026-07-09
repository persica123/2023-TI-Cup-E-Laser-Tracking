#include "pid.h"
#include <string.h>

/**
 * @brief PID ?????
 */
void PID_Init(PID_TypeDef *pid, float kp, float ki, float kd, float max_out, float max_i) {
    memset(pid, 0, sizeof(PID_TypeDef)); // ????????
    
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
    pid->max_output = max_out;
    pid->max_integral = max_i;
}

/**
 * @brief ??? PID ??
 * @note  ??: Out = Kp*e(k) + Ki*sum(e) + Kd*(e(k) - e(k-1))
 */
float PID_Calc_Positional(PID_TypeDef *pid, float target, float current) {
    // 1. ??????
    pid->error = target - current;
    
    // 2. ????????????
    pid->integral += pid->error;
    if (pid->integral > pid->max_integral)  pid->integral = pid->max_integral;
    if (pid->integral < -pid->max_integral) pid->integral = -pid->max_integral;
    
    // 3. ?????????
    float output = (pid->kp * pid->error) + \
                   (pid->ki * pid->integral) + \
                   (pid->kd * (pid->error - pid->last_error));
                   
    // 4. ??????,?????????
    pid->last_error = pid->error;
    
    // 5. ??????
    if (output > pid->max_output)  output = pid->max_output;
    if (output < -pid->max_output) output = -pid->max_output;
    
    return output;
}

/**
 * @brief ??? PID ??
 * @note  ??: delta_Out = Kp*(e(k) - e(k-1)) + Ki*e(k) + Kd*(e(k) - 2*e(k-1) + e(k-2))
 */
float PID_Calc_Incremental(PID_TypeDef *pid, float target, float current) {
    // 1. ??????
    pid->error = target - current;
    
    // 2. ????????
    float delta_output = (pid->kp * (pid->error - pid->last_error)) + \
                         (pid->ki * pid->error) + \
                         (pid->kd * (pid->error - 2.0f * pid->last_error + pid->prev_error));
                         
    // 3. ????????
    pid->prev_error = pid->last_error;
    pid->last_error = pid->error;
    
    // 4. ??? PID ??????? (?????????????????,??????????)
    pid->integral += delta_output; 
    
    // 5. ??????
    if (pid->integral > pid->max_output)  pid->integral = pid->max_output;
    if (pid->integral < -pid->max_output) pid->integral = -pid->max_output;
    
    return pid->integral;
}

/**
 * @brief PID ????
 */
void PID_Reset(PID_TypeDef *pid) {
    pid->error = 0.0f;
    pid->last_error = 0.0f;
    pid->prev_error = 0.0f;
    pid->integral = 0.0f;
}
