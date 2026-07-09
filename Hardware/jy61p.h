#ifndef __JY61P_H
#define __JY61P_H

#include "main.h"
#include "usart.h"
 
#define ACC_UPDATE		0x01
#define GYRO_UPDATE		0x02
#define ANGLE_UPDATE	0x04
#define MAG_UPDATE		0x08
#define READ_UPDATE		0x80
 
void AutoScanSensor(void);
void SensorUartSend(uint8_t *p_data, uint32_t uiSize);
void CopeSensorData(uint32_t uiReg, uint32_t uiRegNum);
void gyroscope_Init(void);
void gyroscope_scan(void);
//????????
void Acc_Cali(void);
//?????????
void Acc_Cali_nblock(void);
//???Z???
int32_t Z_reset_to_zero(void);
//????Z???
int32_t Z_reset_to_zero_nblock(void);
 
//???????????
extern float fAcc[3], fGyro[3], fAngle[3], fYaw;
extern uint8_t gyroscope_flag;
 
 
#endif
 
