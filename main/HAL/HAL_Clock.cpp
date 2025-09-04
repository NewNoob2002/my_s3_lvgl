#include "HAL.h"
#include "ESP32Time.h"

static ESP32Time *rtc = nullptr;


void HAL::Clock_Init()
{
    if(rtc == nullptr)
        rtc = new ESP32Time();
}

void HAL::Clock_GetInfo(Clock_Info_t *info)
{
    if(rtc == nullptr)
        return;
    struct tm timeinfo;
    timeinfo = rtc->getTimeStruct();
    info->year = timeinfo.tm_year;
    info->month = timeinfo.tm_mon;
    info->day = timeinfo.tm_mday;
    info->hour = timeinfo.tm_hour;
    info->minute = timeinfo.tm_min;
    info->second = timeinfo.tm_sec;
    info->millisecond = 0;
}

void HAL::Clock_SetInfo(const Clock_Info_t* info)
{
    if(rtc == nullptr)
        return;
    rtc->setTime(info->second, info->minute, info->hour, info->day, info->month, info->year);
}

