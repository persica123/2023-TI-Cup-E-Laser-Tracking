#ifndef __KEY_H
#define __KEY_H

#include "main.h"

#ifndef KEY_GPIO_Port
#define KEY_GPIO_Port GPIOD
#define KEY_Pin       GPIO_PIN_0
#endif

void Key_Init(void);
void Key_Scan(void);

#endif
