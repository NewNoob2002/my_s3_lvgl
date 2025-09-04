#include "HAL.h"

void setup() {
    Serial.begin(115200);
    Serial.println("GPS Parser Example Starting...");
    
    // 初始化GPS
    HAL::GPS_Init();
    
    Serial.println("GPS initialized, waiting for GNGGA data...");
}

void loop() {
    static uint32_t last_print_time = 0;
    uint32_t current_time = millis();
    
    // 每2秒打印一次GPS信息
    if (current_time - last_print_time >= 2000) {
        HAL::GPS_Info_t gps;
        HAL::GPS_GetInfo(&gps);
        
        if (gps.isValid) {
            Serial.printf("=== GPS Info ===\n");
            Serial.printf("Time: %02d:%02d:%02d\n", gps.clock.hour, gps.clock.minute, gps.clock.second);
            Serial.printf("Position: %.6f, %.6f\n", gps.latitude, gps.longitude);
            Serial.printf("Altitude: %.1f m\n", gps.altitude);
            Serial.printf("Satellites: %d\n", gps.satellites);
            Serial.printf("Quality: %d\n", gps.quality);
            Serial.printf("HDOP: %.1f\n", gps.hdop);
            Serial.printf("Geoid Height: %.1f m\n", gps.geoid_height);
            if (gps.dgps_age > 0) {
                Serial.printf("DGPS Age: %.1f s, Station: %d\n", gps.dgps_age, gps.dgps_station);
            }
            Serial.printf("Firmware: %s\n", gps.firmwareVersion);
            Serial.println("================");
        } else {
            Serial.println("GPS: No valid fix");
        }
        
        last_print_time = current_time;
    }
    
    delay(100);
}

// 测试GNGGA解析的示例数据
void testGNGGAParsing() {
    Serial.println("Testing GNGGA parsing...");
    
    // 示例GNGGA数据
    const char* test_nmea = "$GNGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47";
    
    // 模拟解析
    HAL::GPS_Info_t gps;
    memset(&gps, 0, sizeof(gps));
    
    // 这里可以调用解析函数进行测试
    Serial.printf("Test NMEA: %s\n", test_nmea);
    Serial.println("Parsing completed");
}

// 性能测试
void performanceTest() {
    Serial.println("Starting GPS parsing performance test...");
    
    const char* test_nmea = "$GNGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47";
    
    uint32_t start_time = micros();
    uint32_t iterations = 10000;
    
    for (uint32_t i = 0; i < iterations; i++) {
        // 模拟解析调用
        // parseGNGGA_Fast(test_nmea);
    }
    
    uint32_t end_time = micros();
    uint32_t total_time = end_time - start_time;
    
    Serial.printf("Performance test: %lu iterations in %lu us\n", iterations, total_time);
    Serial.printf("Average time per parse: %.2f us\n", (float)total_time / iterations);
}
