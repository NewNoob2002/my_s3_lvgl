#include <HAL.h>

extern "C" void app_main(void)
{
  HAL::Init();
  while (1)
  {
    delay(1000);
  }
}