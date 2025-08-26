#pragma once

/**
 * \file
 * \brief Internal HAL include file.
 */
#include "Arduino.h"
#include "HAL_Config.h"

namespace HAL
{
    void Init();

    void Update();

    /*Display*/
    bool Display_Init();
    void Display_Update(void *e);
}