#include "HAL_internal.h"
#include "Arduino_GFX_Library.h"
#include "lvgl_port/lv_port.h"
#include "lvgl.h"

#define LCD_BUFFER_SIZE (CONFIG_SCREEN_HOR_RES * CONFIG_SCREEN_VER_RES)
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
    if (!gfx->begin(30 * 1000 * 1000))
    {
        Serial.println("gfx->begin() failed!");
        return false;
    }
    // gfx->setRotation(0);
    gfx->fillScreen(BLACK);
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
        lv_timer_handler();
        vTaskDelay(5);
    }
}

void HAL::Display_SendBuffer(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t *data)
{
#if (LV_COLOR_16_SWAP != 0)
    gfx->draw16bitBeRGBBitmap(x, y, data, w, h);
#else
    gfx->draw16bitRGBBitmap(x, y, data, w, h);
#endif
}

int16_t HAL::Display_GetWidth()
{
    return gfx->width();
}
int16_t HAL::Display_GetHeight()
{
    return gfx->height();
}