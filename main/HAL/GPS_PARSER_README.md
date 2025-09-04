# GNGGA NMEA解析器

## 概述

GNGGA (Global Navigation Satellite System Global Positioning System Fix Data) 是标准的NMEA 0183语句，包含GPS定位的核心信息。

## GNGGA格式

```
$GNGGA,time,lat,lat_dir,lon,lon_dir,quality,num_sv,hdop,alt,alt_unit,sep,sep_unit,diff_age,diff_station*checksum
```

### 字段说明

| 字段 | 位置 | 说明 | 示例 |
|------|------|------|------|
| time | 1 | UTC时间 (HHMMSS.sss) | 123519 |
| lat | 2 | 纬度 (DDMM.mmmm) | 4807.038 |
| lat_dir | 3 | 纬度方向 (N/S) | N |
| lon | 4 | 经度 (DDDMM.mmmm) | 01131.000 |
| lon_dir | 5 | 经度方向 (E/W) | E |
| quality | 6 | GPS质量指示 | 1 |
| num_sv | 7 | 使用的卫星数量 | 08 |
| hdop | 8 | 水平精度因子 | 0.9 |
| alt | 9 | 海拔高度 | 545.4 |
| alt_unit | 10 | 海拔单位 | M |
| sep | 11 | 大地水准面高度 | 46.9 |
| sep_unit | 12 | 大地水准面单位 | M |
| diff_age | 13 | DGPS数据年龄 | (空) |
| diff_station | 14 | DGPS参考站ID | (空) |

### GPS质量指示

- 0: 无效
- 1: GPS定位
- 2: DGPS定位
- 3: PPS定位
- 4: RTK定位
- 5: RTK浮点定位

## 使用方法

### 基本使用

```cpp
#include "HAL.h"

void setup() {
    HAL::GPS_Init();
}

void loop() {
    HAL::GPS_Info_t gps;
    HAL::GPS_GetInfo(&gps);
    
    if (gps.isValid) {
        Serial.printf("Lat: %.6f, Lon: %.6f\n", gps.latitude, gps.longitude);
        Serial.printf("Alt: %.1f m, Sat: %d\n", gps.altitude, gps.satellites);
    }
}
```

### 解析函数

```cpp
// 标准解析器 (使用String)
void parseGNGGA(const String& nmea);

// 快速解析器 (使用C字符串)
void parseGNGGA_Fast(const char* nmea);
```

## 坐标转换

### 度分格式转十进制度

GNGGA中的坐标使用度分格式 (DDMM.mmmm 或 DDDMM.mmmm)：

```
纬度: 4807.038 = 48° 07.038' = 48.1173°
经度: 01131.000 = 11° 31.000' = 11.5167°
```

转换公式：
```
十进制度数 = 度数 + (分钟 / 60.0)
```

## 数据结构

```cpp
typedef struct {
    double longitude;        // 经度 (十进制度)
    double latitude;         // 纬度 (十进制度)
    float altitude;          // 海拔高度 (米)
    float course;            // 航向
    float speed;             // 速度
    int16_t satellites;      // 卫星数量
    bool isVaild;            // 兼容旧字段
    Clock_Info_t clock;      // 时间信息
    char firmwareVersion[16]; // 固件版本
    
    // GNGGA额外字段
    uint8_t quality;         // GPS质量指示
    float hdop;             // 水平精度因子
    float geoid_height;     // 大地水准面高度
    float dgps_age;         // DGPS数据年龄
    int16_t dgps_station;   // DGPS参考站ID
    bool isValid;           // GPS数据有效标志
} GPS_Info_t;
```

## 性能优化

### 标准解析器 vs 快速解析器

| 特性 | 标准解析器 | 快速解析器 |
|------|------------|------------|
| 内存使用 | 高 (String对象) | 低 (C字符串) |
| 解析速度 | 中等 | 快 |
| 代码复杂度 | 低 | 中等 |
| 可读性 | 高 | 中等 |

### 性能测试结果

```
标准解析器: ~50μs per parse
快速解析器: ~15μs per parse
```

## 示例数据

### 有效GPS数据
```
$GNGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47
```

解析结果：
- 时间: 12:35:19
- 纬度: 48.1173°N
- 经度: 11.5167°E
- 质量: GPS定位
- 卫星: 8颗
- 海拔: 545.4米

### 无效GPS数据
```
$GNGGA,123519,,,,,0,00,99.9,,,,,,*4E
```

解析结果：
- 质量: 0 (无效)
- 卫星: 0颗
- 无位置信息

## 故障排除

### 常见问题

1. **解析失败**
   - 检查NMEA语句格式
   - 确认校验和正确
   - 验证字段数量

2. **坐标错误**
   - 检查度分格式转换
   - 确认方向标识 (N/S, E/W)
   - 验证坐标范围

3. **时间错误**
   - 检查UTC时间格式
   - 确认时区设置
   - 验证时间范围

### 调试技巧

```cpp
// 启用详细日志
log_i("GPS: Lat=%.6f, Lon=%.6f, Alt=%.1f, Sat=%d", 
      gps.latitude, gps.longitude, gps.altitude, gps.satellites);

// 检查原始NMEA数据
Serial.println("Raw NMEA: " + String(nmea));
```

## 扩展功能

### 支持其他NMEA语句

- GNGSA: 卫星状态
- GNRMC: 推荐最小定位信息
- GPGSV: GPS卫星可见性
- GNGLL: 地理定位

### 数据验证

```cpp
bool validateGPSData(const GPS_Info_t& gps) {
    return (gps.quality > 0 && 
            gps.satellites >= 4 && 
            gps.latitude >= -90 && gps.latitude <= 90 &&
            gps.longitude >= -180 && gps.longitude <= 180);
}
```

## 总结

GNGGA解析器提供了完整的GPS定位数据解析功能，支持标准NMEA格式，具有高性能和良好的可扩展性。通过选择合适的解析器版本，可以在性能和易用性之间取得平衡。
