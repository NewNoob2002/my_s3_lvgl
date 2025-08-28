#include "HAL_internal.h"

present_devices present;
online_devices online;
settings systemSettings;

void checkDevices() {
    present.microSd = true;
    present.display = true;
}

void HAL::Init()
{
    checkDevices();
    Serial.begin(115200);
    HAL::SD_Init();
    HAL::Display_Init();
}