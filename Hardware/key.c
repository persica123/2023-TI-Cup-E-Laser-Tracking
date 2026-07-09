#include "key.h"
#include "control.h"      // ????
#include "main.h"

#define KEY_DEBOUNCE_MS 20U

static SystemMode next_mode = MODE_TASK_1;   // ?? SystemMode ??
static GPIO_PinState last_sample = GPIO_PIN_SET;
static GPIO_PinState stable_state = GPIO_PIN_SET;
static uint32_t last_change_tick = 0;
static uint8_t press_handled = 0;

void Key_Init(void)
{
    next_mode = MODE_TASK_1;
    last_sample = GPIO_PIN_SET;
    stable_state = GPIO_PIN_SET;
    last_change_tick = HAL_GetTick();
    press_handled = 0;
}

void Key_Scan(void)
{
    GPIO_PinState sample = HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin);
    uint32_t now = HAL_GetTick();

    if (sample != last_sample) {
        last_sample = sample;
        last_change_tick = now;
        return;
    }

    if ((now - last_change_tick) < KEY_DEBOUNCE_MS) {
        return;
    }

    if (sample != stable_state) {
        stable_state = sample;
        if (stable_state == GPIO_PIN_SET) {
            press_handled = 0;
        }
    }

    if (stable_state == GPIO_PIN_RESET && !press_handled) {
        Control_SetMode(next_mode);

        next_mode++;
        if (next_mode > MODE_TASK_4) {
            next_mode = MODE_TASK_1;
        }
        press_handled = 1;
    }
}