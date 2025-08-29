#include "HAL_internal.h"

present_devices present;
online_devices online;
settings systemSettings;

void checkDevices() {
    present.microSd = true;
    present.display = true;
}

void print_heap_info(bool force_print) {
    if (force_print) {
    printf("Heap info:\n");
    printf("Free heap: %d\n", heap_caps_get_free_size(MALLOC_CAP_DMA | MALLOC_CAP_8BIT));
    printf("Total heap: %d\n", heap_caps_get_total_size(MALLOC_CAP_DMA | MALLOC_CAP_8BIT));
    printf("Largest free block: %d\n", heap_caps_get_largest_free_block(MALLOC_CAP_DMA | MALLOC_CAP_8BIT));
    printf("Total SPIRAM: %d\n", heap_caps_get_total_size(MALLOC_CAP_SPIRAM));
    printf("Free SPIRAM: %d\n", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
    printf("Largest free block SPIRAM: %d\n", heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));
    }
}

void HAL::Init()
{
    checkDevices();
    Serial.begin(115200);
    HAL::SD_Init();
    HAL::Display_Init();
}