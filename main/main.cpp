#include <Arduino.h>
#include <Arduino_GFX_Library.h>

#include "gnss.h"

Arduino_DataBus *bus = new Arduino_ESP32SPIDMA(11 /* DC */, 10 /* CS */, 12, 13, -1);
Arduino_GFX *gfx = new Arduino_ST7789(bus, 1 /* RST */, 1 /* rotation */, true /* IPS */, 170 /* width */, 320 /* height */, 35 /* col offset 1 */, 0 /* row offset 1 */, 35 /* col offset 2 */, 0 /* row offset 2 */);

extern "C" void app_main(void)
{
    Serial.begin(115200);
    if (!gfx->begin())
    {
        Serial.println("gfx->begin() failed!");
    }
  gfx->fillScreen(RED);
  gfx->setTextSize(2);
#ifdef DF_GFX_BL
  pinMode(DF_GFX_BL, OUTPUT);
  digitalWrite(DF_GFX_BL, HIGH);
#endif
  gnssTaskBegin();
    while (1)
    {
        delay(100);
    }
}