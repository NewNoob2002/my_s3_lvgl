#include <Arduino.h>
#include <Arduino_GFX_Library.h>

Arduino_DataBus *bus = new Arduino_ESP32SPIDMA(11 /* DC */, 10 /* CS */, 12, 13, -1);
Arduino_GFX *gfx = new Arduino_ST7789(bus, 1 /* RST */, 1 /* rotation */, true /* IPS */, 170 /* width */, 320 /* height */, 35 /* col offset 1 */, 0 /* row offset 1 */, 35 /* col offset 2 */, 0 /* row offset 2 */);

#define rgb565(r, g, b) (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3))

extern "C" void app_main(void)
{
    Serial.begin(115200);
    if (!gfx->begin())
    {
        Serial.println("gfx->begin() failed!");
    }
  gfx->fillScreen(RED);
  gfx->setCursor(10, 10);
  gfx->setTextColor(RGB565_WHITE);
  gfx->println("Hello World!");
#ifdef DF_GFX_BL
  pinMode(DF_GFX_BL, OUTPUT);
  digitalWrite(DF_GFX_BL, HIGH);
#endif
    while (1)
    {
        delay(100);
    }
}