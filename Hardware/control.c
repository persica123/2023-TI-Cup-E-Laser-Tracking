#include "control.h"
#include "motor.h"
#include "track.h"
#include "pid.h"
#include "encoder.h"
#include "jy61p.h"
#include <math.h>
#include <string.h>

#define JY61P_GetYaw()  (fYaw)

/* ================= ???? ================= */
#define WHEEL_CIRCUMFERENCE   (PI * WHEEL_DIAMETER * 100.0f)  // cm,????,?20.42cm
#define PULSE_PER_CM          (PULSE_PER_ROUND / WHEEL_CIRCUMFERENCE) // ?86.2??/cm

// ??????(??:cm)???????,????????
#define DIST_AB  100.0f   // A?B????(???,????????)
#define DIST_BC  125.6f   // B?C????(???? = p*R = 3.14*40 ?125.6cm)
#define DIST_CD  100.0f   // C?D????
#define DIST_DA  125.6f   // D?A????
#define DIST_AC  160.0f   // A?C????(??,???,????)

// ????
#define STRAIGHT_TARGET_SPEED   100.0f  // ?????? RPM
#define ARC_TARGET_SPEED        80.0f   // ?????? RPM

// ???? PID ??(??????)
#define HEADING_KP  2.5f
#define HEADING_KD  0.5f
#define MAX_STEER_HEADING   50.0f   // ??????PWM???

// ????????????
#define ARC_KP      1.2f
#define ARC_MAX_STEER  60.0f

// ??PID??(??????,??????)
PID_TypeDef pid_left, pid_right;
#define encoder_left   Encoder_L
#define encoder_right  Encoder_R

/* ================= ???? ================= */
SystemMode g_system_mode = MODE_IDLE;

// ????(????????????????)
static const PathSegment task1_path[] = {
    {PATH_STRAIGHT, DIST_AB, STRAIGHT_TARGET_SPEED},
    {PATH_STRAIGHT, 0, 0}  // ????
};

static const PathSegment task2_path[] = {
    {PATH_STRAIGHT, DIST_AB, STRAIGHT_TARGET_SPEED},   // A->B
    {PATH_ARC_LEFT, 0, ARC_TARGET_SPEED},              // B->C(???,????????)
    {PATH_STRAIGHT, DIST_CD, STRAIGHT_TARGET_SPEED},   // C->D
    {PATH_ARC_LEFT, 0, ARC_TARGET_SPEED},              // D->A(???)
    {PATH_STRAIGHT, 0, 0}  // ??
};

static const PathSegment task3_path[] = {
    {PATH_STRAIGHT, DIST_AC, STRAIGHT_TARGET_SPEED},   // A->C(???)
    {PATH_ARC_LEFT, 0, ARC_TARGET_SPEED},              // C->B(??,?????)
    {PATH_STRAIGHT, DIST_CD, STRAIGHT_TARGET_SPEED},   // B->D
    {PATH_ARC_LEFT, 0, ARC_TARGET_SPEED},              // D->A
    {PATH_STRAIGHT, 0, 0}
};

/* ================= ????? ================= */
static const PathSegment *current_path = NULL;
static uint8_t segment_index = 0;
static float segment_start_distance = 0.0f;    // ?????????(cm)
static uint8_t task4_remaining_loops = 4;       // ??4????
static uint8_t task4_inner_loop_index = 0;      // ??4???????

// ??????
static float target_heading = 0.0f;             // ?????(?)
static float last_heading_error = 0.0f;

/* ================= ?????? ================= */
static void RunPathSegment(const PathSegment *seg);
static void StraightControl(float target_speed_rpm, float target_distance_cm);
static void ArcControl(PathType arc_type, float target_speed_rpm);
static void UpdateOdometry(void);
static void ResetOdometry(void);
static void StopAndReset(void);
static void StartNextSegment(void);

/* ================= ???(????) ================= */
static float total_distance_cm = 0.0f;   // ???????????(cm)
static float last_left_pulses = 0, last_right_pulses = 0;

// ?????:?????????????????
static void UpdateOdometry(void)
{
    static int64_t last_total_pulses = 0;
    int64_t current_total_pulses = (encoder_left.total_pulses + encoder_right.total_pulses) / 2;
    int64_t delta_pulses = current_total_pulses - last_total_pulses;
    float delta_cm = (float)delta_pulses / PULSE_PER_CM;
    total_distance_cm += delta_cm;
    last_total_pulses = current_total_pulses;
}

static void ResetOdometry(void)
{
    total_distance_cm = 0.0f;
    // ?????????(??:??????,????????total_pulses???)
    encoder_left.total_pulses = 0;
    encoder_right.total_pulses = 0;
    // ??last_total_pulses???????UpdateOdometry????
}

float Get_Travel_Distance(void)
{
    return total_distance_cm;
}

/* ================= ????(???? + ????) ================= */
static void StraightControl(float target_speed_rpm, float target_distance_cm)
{
    // 1. ??????????
    float travelled = total_distance_cm - segment_start_distance;
    if (travelled >= target_distance_cm) {
        // ????,????????
        Motor_Stop();
        StartNextSegment();
        return;
    }
    
    // 2. ????:????????(???????????)
    static uint8_t heading_initialized = 0;
    if (!heading_initialized) {
        target_heading = JY61P_GetYaw();
        heading_initialized = 1;
        last_heading_error = 0;
    }
    float current_heading = JY61P_GetYaw();
    float heading_error = target_heading - current_heading;
    // ?????? -180~180
    if (heading_error > 180.0f) heading_error -= 360.0f;
    if (heading_error < -180.0f) heading_error += 360.0f;
    
    // PD ???????
    float steer = HEADING_KP * heading_error + HEADING_KD * (heading_error - last_heading_error);
    last_heading_error = heading_error;
    if (steer > MAX_STEER_HEADING) steer = MAX_STEER_HEADING;
    if (steer < -MAX_STEER_HEADING) steer = -MAX_STEER_HEADING;
    
    // 3. ????(???? RPM)
    float left_target  = target_speed_rpm - steer;
    float right_target = target_speed_rpm + steer;
    
    float left_pwm = PID_Calc_Positional(&pid_left,  left_target,  encoder_left.current_rpm);
    float right_pwm = PID_Calc_Positional(&pid_right, right_target, encoder_right.current_rpm);
    
    left_pwm  = (left_pwm  > 100.0f) ? 100.0f : (left_pwm  < -100.0f ? -100.0f : left_pwm);
    right_pwm = (right_pwm > 100.0f) ? 100.0f : (right_pwm < -100.0f ? -100.0f : right_pwm);
    
    Motor_Set_Left((int16_t)left_pwm);
    Motor_Set_Right((int16_t)right_pwm);
}

/* ================= ??????(???????) ================= */
static void ArcControl(PathType arc_type, float target_speed_rpm)
{
    // ?? Track_Get_Error() ????(-5~+5)
    float error = Track_Get_Error();
    // ??????????????
    if (arc_type == PATH_ARC_LEFT) {
        // ???,???????,???????????????????
        // ??????????????????????,???????
        // ???????,??? -1
    }
    // ??????
    float steer = error * ARC_KP;
    if (steer > ARC_MAX_STEER) steer = ARC_MAX_STEER;
    if (steer < -ARC_MAX_STEER) steer = -ARC_MAX_STEER;
    
    // ????
    float left_target  = target_speed_rpm - steer;
    float right_target = target_speed_rpm + steer;
    
    float left_pwm = PID_Calc_Positional(&pid_left,  left_target,  encoder_left.current_rpm);
    float right_pwm = PID_Calc_Positional(&pid_right, right_target, encoder_right.current_rpm);
    
    left_pwm  = (left_pwm  > 100.0f) ? 100.0f : (left_pwm  < -100.0f ? -100.0f : left_pwm);
    right_pwm = (right_pwm > 100.0f) ? 100.0f : (right_pwm < -100.0f ? -100.0f : right_pwm);
    
    Motor_Set_Left((int16_t)left_pwm);
    Motor_Set_Right((int16_t)right_pwm);
    
    // ?????????:???????????????,????????????????
    // ??????:?????????????(???????????????)
    // ??????????:????????????,?????????????
    // ???????(DIST_BC,DIST_DA),???????????distance_cm?
    // ??:?PathSegment???????????,??????????
}

/* ================= ?????(????) ================= */
static void RunPathSegment(const PathSegment *seg)
{
    if (seg->type == PATH_STRAIGHT) {
        StraightControl(seg->target_speed_rpm, seg->distance_cm);
    } else {
        // ????(??????)
        float travelled = total_distance_cm - segment_start_distance;
        if (travelled >= seg->distance_cm) {
            Motor_Stop();
            StartNextSegment();
            return;
        }
        // ???????
        float error = Track_Get_Error();
        // ????????????????(????,???????)
        if (seg->type == PATH_ARC_LEFT) {
            // ??????????????????,Track_Get_Error????????0
        }
        float steer = error * ARC_KP;
        if (steer > ARC_MAX_STEER) steer = ARC_MAX_STEER;
        if (steer < -ARC_MAX_STEER) steer = -ARC_MAX_STEER;
        
        float left_target  = seg->target_speed_rpm - steer;
        float right_target = seg->target_speed_rpm + steer;
        float left_pwm = PID_Calc_Positional(&pid_left,  left_target,  encoder_left.current_rpm);
        float right_pwm = PID_Calc_Positional(&pid_right, right_target, encoder_right.current_rpm);
        left_pwm  = (left_pwm  > 100.0f) ? 100.0f : (left_pwm  < -100.0f ? -100.0f : left_pwm);
        right_pwm = (right_pwm > 100.0f) ? 100.0f : (right_pwm < -100.0f ? -100.0f : right_pwm);
        Motor_Set_Left((int16_t)left_pwm);
        Motor_Set_Right((int16_t)right_pwm);
    }
}

/* ================= ????????? ================= */
static void StartNextSegment(void)
{
    segment_index++;
    if (current_path[segment_index].distance_cm == 0 && current_path[segment_index].type == PATH_STRAIGHT) {
        // ??????
        if (g_system_mode == MODE_TASK_4) {
            task4_remaining_loops--;
            if (task4_remaining_loops > 0) {
                // ????TASK_3???
                segment_index = 0;
                segment_start_distance = total_distance_cm;
                // ?????????
                target_heading = JY61P_GetYaw();
                last_heading_error = 0;
                return;
            } else {
                StopAndReset();
                g_system_mode = MODE_IDLE;
                return;
            }
        } else {
            StopAndReset();
            g_system_mode = MODE_IDLE;
            return;
        }
    }
    // ????:??????,??????
    segment_start_distance = total_distance_cm;
    // ??????,????????,?StraightControl??????
}

static void StopAndReset(void)
{
    Motor_Stop();
    // ???????????(??)
}

/* ================= ???? ================= */
void Control_Init(void)
{
    Motor_Init();
    PID_Init(&pid_left,  2.0f, 0.5f, 0.1f, 100.0f, 50.0f);
    PID_Init(&pid_right, 2.0f, 0.5f, 0.1f, 100.0f, 50.0f);
    // ????????????
    ResetOdometry();
    g_system_mode = MODE_IDLE;
}

void Control_Task(void)
{
    UpdateOdometry();   // ?????
    
    if (g_system_mode == MODE_IDLE) {
        return;
    }
    
    if (current_path == NULL) {
        // ??????????
        switch (g_system_mode) {
            case MODE_TASK_1:
                current_path = task1_path;
                break;
            case MODE_TASK_2:
                current_path = task2_path;
                break;
            case MODE_TASK_3:
                current_path = task3_path;
                break;
            case MODE_TASK_4:
                current_path = task3_path;  // ??4??task3???
                task4_remaining_loops = 4;
                break;
            default:
                return;
        }
        segment_index = 0;
        segment_start_distance = total_distance_cm;
        // ?????????(task1/2/3???????????)
        // ??:task2?task3????????distance_cm,?????
    }
    
    const PathSegment *seg = &current_path[segment_index];
    RunPathSegment(seg);
}

void Control_SetMode(SystemMode mode)
{
    if (mode == g_system_mode) return;
    // ??????
    Motor_Stop();
    // ?????
    current_path = NULL;
    segment_index = 0;
    ResetOdometry();
    g_system_mode = mode;
    // ?????4,??????
    if (mode == MODE_TASK_4) {
        task4_remaining_loops = 4;
    }
}

void Control_Stop(void)
{
    Motor_Stop();
    g_system_mode = MODE_IDLE;
    current_path = NULL;
}
