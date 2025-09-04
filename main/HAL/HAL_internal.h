#pragma once

/**
 * \file
 * \brief Internal HAL include file.
 */
#include "Arduino.h"
#include "HAL_Define.h"
#include "HAL_Config.h"

namespace HAL
{
    void Init();

    void Update();

    /*Display*/
    bool Display_Init();
    void Display_Update(void *e);

    void Display_SendBuffer(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t *data);
    int16_t Display_GetWidth();
    int16_t Display_GetHeight();

    /*Backlight*/
    void Backlight_Init();
    void Backlight_SetGradual(uint16_t target, uint16_t time);
    uint16_t Backlight_GetBrightness();
    void Backlight_SetValue(int16_t val);

    /*SD*/
    bool SD_Init();

    /*Encoder*/
    void Encoder_Init();
    void Encoder_Update(void *e);
    int32_t Encoder_GetDiff();
    bool Encoder_GetIsPush();
    void Encoder_SetEnable(bool en);
    uint32_t Encoder_GetEventCount();
    uint32_t Encoder_GetLastEventTime();
    uint8_t Encoder_GetCurrentState();
    uint8_t Encoder_GetPreviousState();

    /*GPS*/
    void GPS_Init();
    void GPS_Update(void *e);
    void GPS_GetInfo(GPS_Info_t *info);
}

void print_heap_info(bool force_print);