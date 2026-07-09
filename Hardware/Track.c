#include "track.h"

// ???8?????? (????: X1 ? X8)
static const float sensor_weights[8] = {-5.0f, -4.0f, -2.0f, -1.0f, 1.0f, 2.0f, 4.0f, 5.0f};
static float last_valid_error = 0.0f; 

/**
 * @brief  ????????? (?? STM32F407)
 * @note   HAL ??? delay_us,?????? CPU ?????
 */
static void delay_us(uint32_t us) {
    // STM32F407 ????? 168MHz???????????????
    // SystemCoreClock ???????????
    uint32_t delay = (SystemCoreClock / 4000000) * us; 
    while (delay--) {
        __NOP(); // CPU ????
    }
}

/**
 * @brief  ?? AD0~AD2 ???????????? (0~7)
 */
static void _select_channel(uint8_t channel) {
    // ?? channel ? bit0, bit1, bit2 ??????
    HAL_GPIO_WritePin(SENSOR_AD0_PORT, SENSOR_AD0_PIN, (channel & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(SENSOR_AD1_PORT, SENSOR_AD1_PIN, ((channel >> 1) & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(SENSOR_AD2_PORT, SENSOR_AD2_PIN, ((channel >> 2) & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}



float Track_Get_Error(void) {
    uint8_t sensor_values[8] = {0};
    float weighted_sum = 0.0f;
    int active_sensors = 0;

    // 1. ???? 8 ???????
    for(uint8_t i = 0; i < 8; i++) {
        _select_channel(i);  // ????
        delay_us(50);        // ????? 50us ??????
        
        // ?? OUT ????
        if (HAL_GPIO_ReadPin(SENSOR_OUT_PORT, SENSOR_OUT_PIN) == LINE_RAW_VALUE) {
            sensor_values[i] = 1; // ??
        } else {
            sensor_values[i] = 0; // ???
        }
    }

    // 2. ???????? (?????????????)
    for (int i = 0; i < 8; i++) {
        if (sensor_values[i] == 1) { 
            weighted_sum += sensor_weights[i];
            active_sensors++;
        }
    }

    // 3. ????
    if (active_sensors == 0) {
        return last_valid_error; 
    }

    float current_error = weighted_sum / active_sensors;
    last_valid_error = current_error; 

    return current_error;
}


uint8_t Track_Is_AllWhite(void) {
    for (uint8_t i = 0; i < 8; i++) {
        _select_channel(i);
        delay_us(50);
        if (HAL_GPIO_ReadPin(SENSOR_OUT_PORT, SENSOR_OUT_PIN) == LINE_RAW_VALUE)
            return 0;   
    }
    return 1;
}
/**
 * @brief  ?????? A?B?C?D ?? (????)
 * @retval 1: ????, 0: ???
 */
uint8_t Track_Is_Node_Detected(void) {
    uint8_t left_hit = 0;
    uint8_t right_hit = 0;

    // 1. ??????? (?? 0)
    _select_channel(0);
    delay_us(50);
    if (HAL_GPIO_ReadPin(SENSOR_OUT_PORT, SENSOR_OUT_PIN) == LINE_RAW_VALUE) {
        left_hit = 1;
    }

    // 2. ??????? (?? 7)
    _select_channel(7);
    delay_us(50);
    if (HAL_GPIO_ReadPin(SENSOR_OUT_PORT, SENSOR_OUT_PIN) == LINE_RAW_VALUE) {
        right_hit = 1;
    }

    // 3. ?????????,????0?1?????,??6?7???????
    // ???????0?7??????:
    if (left_hit && right_hit) {
        return 1; // ????,?????
    }

    return 0; // ?????
}
