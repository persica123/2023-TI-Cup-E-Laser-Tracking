#ifndef __TRACK_H
#define __TRACK_H

#include "main.h"

// 1: ????????/??  0: ????????/??
#define LINE_RAW_VALUE 1 

/* ================= ????????? ================= */
// ??????,???? PA0~PA3,?????? CubeMX ????
#define SENSOR_AD0_PORT GPIOE
#define SENSOR_AD0_PIN  GPIO_PIN_7

#define SENSOR_AD1_PORT GPIOE
#define SENSOR_AD1_PIN  GPIO_PIN_9

#define SENSOR_AD2_PORT GPIOE
#define SENSOR_AD2_PIN  GPIO_PIN_11

#define SENSOR_OUT_PORT GPIOE
#define SENSOR_OUT_PIN  GPIO_PIN_13



float Track_Get_Error(void);
uint8_t Track_Is_AllWhite(void);
uint8_t Track_Is_Node_Detected(void);
#endif
