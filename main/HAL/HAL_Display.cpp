#include "HAL_internal.h"
#include "Arduino_GFX_Library.h"
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

lv_disp_draw_buf_t draw_buf;
lv_color_t *disp_draw_buf;
lv_color_t *disp_draw_buf2;
lv_disp_drv_t disp_drv;

/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
#ifndef DIRECT_RENDER_MODE
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

#if (LV_COLOR_16_SWAP != 0)
    gfx->draw16bitBeRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
#else
    gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
#endif
#endif // #ifndef DIRECT_RENDER_MODE

    lv_disp_flush_ready(disp_drv);
}

bool HAL::Display_Init()
{
    if (!gfx->begin(30 * 1000 * 1000))
    {
        Serial.println("gfx->begin() failed!");
        return false;
    }
    gfx->fillScreen(BLACK);
#ifdef CONFIG_SCREEN_BLK_PIN
    pinMode(CONFIG_SCREEN_BLK_PIN, OUTPUT);
    digitalWrite(CONFIG_SCREEN_BLK_PIN, HIGH);
#endif
    lv_init();
    disp_draw_buf = (lv_color_t *)heap_caps_malloc(LCD_BUFFER_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA | MALLOC_CAP_8BIT);
    disp_draw_buf2 = (lv_color_t *)heap_caps_malloc(LCD_BUFFER_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA | MALLOC_CAP_8BIT);
    if (!disp_draw_buf)
    {
        // remove MALLOC_CAP_INTERNAL flag try again
        disp_draw_buf = (lv_color_t *)heap_caps_malloc(LCD_BUFFER_SIZE * 2, MALLOC_CAP_DMA | MALLOC_CAP_8BIT);
    }
    if (!disp_draw_buf || !disp_draw_buf2)
    {
        Serial.println("LVGL disp_draw_buf allocate failed!");
        return false;
    }
    else
    {
        lv_disp_draw_buf_init(&draw_buf, disp_draw_buf, disp_draw_buf2, LCD_BUFFER_SIZE);

        /* Initialize the display */
        lv_disp_drv_init(&disp_drv);
        /* Change the following line to your display resolution */
        disp_drv.hor_res = gfx->width();
        disp_drv.ver_res = gfx->height();
        disp_drv.flush_cb = my_disp_flush;
        disp_drv.draw_buf = &draw_buf;
#ifdef DIRECT_RENDER_MODE
        disp_drv.direct_mode = true;
#endif
        lv_disp_drv_register(&disp_drv);
    }
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