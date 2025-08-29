#include "lvgl.h"
#include "HAL.h"

#define SCREEN_BUFFER_SIZE (CONFIG_SCREEN_HOR_RES * CONFIG_SCREEN_VER_RES)

lv_disp_draw_buf_t draw_buf;
lv_disp_drv_t disp_drv;


void my_disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
#ifndef DIRECT_RENDER_MODE
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    HAL::Display_SendBuffer(area->x1, area->y1, w, h, (uint16_t *)color_p);
#endif // #ifndef DIRECT_RENDER_MODE
    lv_disp_flush_ready(disp_drv);
}

void lv_port_disp_init(void)
{
    // static lv_color_t lv_disp_buf1[SCREEN_BUFFER_SIZE];
    // static lv_color_t lv_disp_buf2[SCREEN_BUFFER_SIZE];
    // lv_color_t *disp_draw_buf = (lv_color_t *)malloc(SCREEN_BUFFER_SIZE * sizeof(lv_color_t));
    // lv_color_t *disp_draw_buf2 = (lv_color_t *)malloc(SCREEN_BUFFER_SIZE * sizeof(lv_color_t));
    lv_color_t *disp_draw_buf = (lv_color_t *)heap_caps_malloc(SCREEN_BUFFER_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA | MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    lv_color_t *disp_draw_buf2 = (lv_color_t *)heap_caps_malloc(SCREEN_BUFFER_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA |MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    lv_disp_draw_buf_init(&draw_buf, disp_draw_buf, disp_draw_buf2, SCREEN_BUFFER_SIZE);

    /* Initialize the display */
    lv_disp_drv_init(&disp_drv);
    /* Change the following line to your display resolution */
    disp_drv.hor_res = HAL::Display_GetWidth();
    disp_drv.ver_res = HAL::Display_GetHeight();
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
#ifdef DIRECT_RENDER_MODE
    disp_drv.direct_mode = true;
#endif
    lv_disp_drv_register(&disp_drv);
}