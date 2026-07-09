#include "jy61p.h"
#include "wit_c_sdk.h"
#include "REG.h"



uint32_t uiBuad = 115200;
uint8_t ucRxData = 0;
float fAcc[3], fGyro[3], fAngle[3], fYaw;
int i;
char s_cDataUpdate = 0;
 
uint8_t gyroscope_flag;

static void JY61P_DelayMs(uint16_t ms)
{
	HAL_Delay(ms);
}
 
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if(huart->Instance==UART5)
  {
      WitSerialDataIn(ucRxData);
      UART_Start_Receive_IT(huart, &ucRxData, 1);
  }
}
 
void SensorUartSend(uint8_t *p_data, uint32_t uiSize)
{
    HAL_UART_Transmit(&huart4, p_data, uiSize, uiSize*4);
}
void CopeSensorData(uint32_t uiReg, uint32_t uiRegNum)
{
	int i;
    for(i = 0; i < uiRegNum; i++)
    {
        switch(uiReg)
        {
            case AZ:
				s_cDataUpdate |= ACC_UPDATE;
            break;
            case GZ:
				s_cDataUpdate |= GYRO_UPDATE;
            break;
            case HZ:
				s_cDataUpdate |= MAG_UPDATE;
            break;
            case Yaw:
				s_cDataUpdate |= ANGLE_UPDATE;
            break;
            default:
				s_cDataUpdate |= READ_UPDATE;
			break;
        }
		uiReg++;
    }
}
 
void AutoScanSensor(void)
{
	const uint32_t c_uiBaud[9] = {4800, 9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600};
	int i, iRetry;
	
	for(i = 0; i < 9; i++)
	{
    uiBuad = c_uiBaud[i];
		iRetry = 2;
		do
		{
			s_cDataUpdate = 0;
			WitReadReg(AX, 3);
			HAL_Delay(100);
			if(s_cDataUpdate != 0)
			{
				return ;
			}
			iRetry--;
		}while(iRetry);		
	}
}
 
void gyroscope_Init(void)
{
	UART_Start_Receive_IT(&huart4, &ucRxData, 1);
	
	WitInit(WIT_PROTOCOL_NORMAL, 0x50);
	WitSerialWriteRegister(SensorUartSend);
	WitRegisterCallBack(CopeSensorData);
	WitDelayMsRegister(JY61P_DelayMs);    // ?HAL_Delay????
	
	AutoScanSensor();
}
 
void gyroscope_scan(void)
{
	if(s_cDataUpdate)
	{
		// ??????
		for(i = 0; i < 3; i++)
		{
			fAcc[i] = sReg[AX+i] / 32768.0f * 16.0f;
			fGyro[i] = sReg[GX+i] / 32768.0f * 2000.0f;
			fAngle[i] = sReg[Roll+i] / 32768.0f * 180.0f;
		}
		if(s_cDataUpdate & ACC_UPDATE)
		{
			s_cDataUpdate &= ~ACC_UPDATE;
		}
		if(s_cDataUpdate & GYRO_UPDATE)
		{
			s_cDataUpdate &= ~GYRO_UPDATE;
		}
		if(s_cDataUpdate & ANGLE_UPDATE)
		{
			fYaw = (float)((unsigned short)sReg[Yaw]) / 32768 * 180.0f;
			s_cDataUpdate &= ~ANGLE_UPDATE;
		}
		if(s_cDataUpdate & MAG_UPDATE)
		{
			s_cDataUpdate &= ~MAG_UPDATE;
		}
		s_cDataUpdate = 0;
	}
}
 
//????????
void Acc_Cali(void)
{
	WitStartAccCali();
	HAL_Delay(6000);
	WitStopAccCali();
}
 
//?????????
void Acc_Cali_nblock(void)
{
	static uint32_t tick1 = 0;
	static uint32_t flag1 = 0;
	switch(flag1)
	{
		case 0:
			WitStartAccCali();
			tick1 = HAL_GetTick();
			flag1 = 1;
			break;
		case 1:
			if(HAL_GetTick() - tick1 >= 6000)	flag1 = 2;
			break;
		case 2:
			WitStopAccCali();
			flag1 = 0;
			gyroscope_flag = 0;
			break;
		default:
			break;
		
	}
}
 
//???Z???(??????)
int32_t Z_reset_to_zero(void)
{
    if(WitWriteReg(KEY, KEY_UNLOCK) != WIT_HAL_OK) return WIT_HAL_ERROR;
    
    HAL_Delay(200);
    
    if(WitWriteReg(CALSW, 0x0004) != WIT_HAL_OK) return WIT_HAL_ERROR;
    
    HAL_Delay(3000);
    
    if(WitWriteReg(SAVE, 0x0000) != WIT_HAL_OK) return WIT_HAL_ERROR;
    
    return WIT_HAL_OK;
}
 
//???Z???(??????)
int32_t Z_reset_to_zero_nblock(void)
{
	static uint32_t tick2 = 0;
	static uint8_t flag2 = 0;
	switch(flag2)
	{
		case 0:
			if(WitWriteReg(KEY, KEY_UNLOCK) != WIT_HAL_OK) return WIT_HAL_ERROR;
			tick2 = HAL_GetTick();
			flag2 = 1;
			break;
		case 1:
			if(HAL_GetTick() - tick2 >= 200)	flag2 = 2;
			break;
		case 2:
			if(WitWriteReg(CALSW, 0x0004) != WIT_HAL_OK) return WIT_HAL_ERROR;
			tick2 = HAL_GetTick();
			flag2 = 3;
			break;
		case 3:
			if(HAL_GetTick() - tick2 >= 3000)	flag2 = 4;
			break;
		case 4:
			if(WitWriteReg(SAVE, 0x0000) != WIT_HAL_OK) return WIT_HAL_ERROR;
			flag2 = 0;
			gyroscope_flag = 0;
			break;
		default:
			break;
		
	}
	return WIT_HAL_OK;
}
 
