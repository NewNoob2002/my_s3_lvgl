#include "HAL.h"

void setup() {
    Serial.begin(115200);
    Serial.println("Encoder Debounce Example Starting...");
    
    // 初始化编码器
    HAL::Encoder_Init();
    
    Serial.println("Encoder initialized with debounce");
    Serial.println("Rotate the encoder to test debounce effect");
}

void loop() {
    static uint32_t last_print_time = 0;
    uint32_t current_time = millis();
    
    // 每500ms打印一次统计信息
    if (current_time - last_print_time >= 500) {
        Serial.printf("Encoder Stats - Events: %lu, LastEvent: %lu us ago\n", 
                      HAL::Encoder_GetEventCount(), 
                      micros() - HAL::Encoder_GetLastEventTime());
        Serial.printf("Current State: %d, Previous State: %d\n", 
                      HAL::Encoder_GetCurrentState(), 
                      HAL::Encoder_GetPreviousState());
        last_print_time = current_time;
    }
    
    // 检查编码器旋转
    int32_t diff = HAL::Encoder_GetDiff();
    if (diff != 0) {
        Serial.printf("Encoder rotated: %ld steps\n", diff);
    }
    
    // 检查按钮
    if (HAL::Encoder_GetIsPush()) {
        Serial.println("Encoder button pressed");
    }
    
    delay(10);
}

// 性能测试函数
void performanceTest() {
    Serial.println("Starting encoder performance test...");
    
    uint32_t start_time = micros();
    uint32_t test_duration = 1000000;  // 1秒
    
    while (micros() - start_time < test_duration) {
        // 模拟高频读取
        HAL::Encoder_GetDiff();
        HAL::Encoder_GetIsPush();
    }
    
    uint32_t end_time = micros();
    uint32_t total_time = end_time - start_time;
    
    Serial.printf("Performance test completed in %lu us\n", total_time);
    Serial.printf("Total encoder events: %lu\n", HAL::Encoder_GetEventCount());
}
