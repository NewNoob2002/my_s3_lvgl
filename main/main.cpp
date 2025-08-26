#include <HAL.h>

extern "C" void app_main(void)
{
  HAL::Init();
  xTaskCreate(HAL::Display_Update, "lvgl", 10*1024, nullptr, 12, nullptr);
  while (1)
  {
    delay(10);
  }
}