#include <HAL.h>
#include <lvgl_port/lv_port.h>

extern "C" void app_main(void)
{
  HAL::Init();
  lv_init();
  lv_port_init();
  xTaskCreatePinnedToCore(HAL::Display_Update, "lvgl", 10*1024, nullptr, 12, nullptr, 1);
  
  while (1)
  {
    if(xSemaphoreTake(lvglSemaphore, 10) == pdPASS)
    {
      /*USER CODE*/
      xSemaphoreGive(lvglSemaphore);
    }
    delay(100);
  }
}