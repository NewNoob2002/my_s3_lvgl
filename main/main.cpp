#include <HAL.h>
#include "APP/App.h"
#include <lvgl_port/lv_port.h>

void setup()
{
  HAL::Init();
  lv_init();
  lv_port_init();
  HAL::Backlight_SetGradual(1023, 1000);

  App_Init();

}

void loop()
{
  while (1)
  {
    HAL::Encoder_Update(nullptr);
    delay(10);
  }
}

extern "C" void app_main(void)
{
  setup();
  xTaskCreatePinnedToCore(HAL::Display_Update, "lvgl", 10 * 1024, nullptr, 12, nullptr, 1);
  loop();
}