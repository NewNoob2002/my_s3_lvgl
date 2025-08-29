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

    /*SD*/
    bool SD_Init();
}

void print_heap_info(bool force_print);