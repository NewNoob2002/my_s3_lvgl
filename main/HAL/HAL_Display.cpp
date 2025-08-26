#include "HAL_internal.h"
#include "Arduino_GFX_Library.h"
#include "logo.h"

Arduino_DataBus *bus = new Arduino_ESP32SPIDMA(CONFIG_SCREEN_DC_PIN /* DC */,
                                               CONFIG_SCREEN_CS_PIN /* CS */,
                                               CONFIG_SCREEN_SCK_PIN,  /*SCK*/
                                               CONFIG_SCREEN_MOSI_PIN, /*MOSI*/
                                               PIN_NOT_DEFINED,        /*MISO*/
                                               CONFIG_SCREEN_SPI_NUM,  /*SPI_NUM*/
                                               CONFIG_SCREEN_SPI_SHARED);
Arduino_GFX *gfx = new Arduino_ST7789(bus,
                                      CONFIG_SCREEN_RST_PIN /* RST */,
                                      CONFIG_SCREEN_ROTATION /* rotation */,
                                      true /* IPS */,
                                      CONFIG_SCREEN_HOR_RES /* width */,
                                      CONFIG_SCREEN_VER_RES /* height */,
                                      CONFIG_SCREEN_OFFSETX1 /* col offset 1 */,
                                      CONFIG_SCREEN_OFFSETY1 /* row offset 1 */,
                                      CONFIG_SCREEN_OFFSETX2 /* col offset 2 */,
                                      CONFIG_SCREEN_OFFSETY2 /* row offset 2 */);


bool HAL::Display_Init()
{
    if (!gfx->begin(30* 1000* 1000))
    {
        Serial.println("gfx->begin() failed!");
        return false;
    }
    gfx->drawBitmap(0, 0, (uint8_t *)gImage_1, 150, 126, GREEN);
#ifdef CONFIG_SCREEN_BLK_PIN
    pinMode(CONFIG_SCREEN_BLK_PIN, OUTPUT);
    digitalWrite(CONFIG_SCREEN_BLK_PIN, HIGH);
#endif
    return true;
}

void HAL::Display_Update(void *e)
{
    while (1)
    {
        vTaskDelay(5);
    }
}