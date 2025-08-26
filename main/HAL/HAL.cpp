#include "HAL_internal.h"

void HAL::Init()
{
    Serial.begin(115200);
    HAL::Display_Init();
}