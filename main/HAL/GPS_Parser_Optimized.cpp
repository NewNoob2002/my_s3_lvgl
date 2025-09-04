// 高效的GNGGA解析器 - 使用C风格字符串处理
#include "HAL.h"

// 快速解析GNGGA - 避免String对象开销
void parseGNGGA_Fast(const char* nmea)
{
    // 跳过$GNGGA,
    const char* ptr = strchr(nmea, ',');
    if (!ptr) return;
    ptr++; // 跳过逗号
    
    int field_index = 0;
    const char* field_start = ptr;
    
    while (*ptr && field_index < 15) {
        if (*ptr == ',' || *ptr == '*') {
            // 处理当前字段
            int field_len = ptr - field_start;
            
            switch (field_index) {
                case 0: // UTC时间 (HHMMSS.sss)
                    if (field_len >= 6) {
                        gpsInfo.clock.hour = (field_start[0] - '0') * 10 + (field_start[1] - '0');
                        gpsInfo.clock.minute = (field_start[2] - '0') * 10 + (field_start[3] - '0');
                        gpsInfo.clock.second = (field_start[4] - '0') * 10 + (field_start[5] - '0');
                    }
                    break;
                    
                case 1: // 纬度 (DDMM.mmmm)
                    if (field_len > 0) {
                        gpsInfo.latitude = parseCoordinateFast(field_start, field_len);
                    }
                    break;
                    
                case 2: // 纬度方向 (N/S)
                    if (field_len > 0 && *field_start == 'S') {
                        gpsInfo.latitude = -gpsInfo.latitude;
                    }
                    break;
                    
                case 3: // 经度 (DDDMM.mmmm)
                    if (field_len > 0) {
                        gpsInfo.longitude = parseCoordinateFast(field_start, field_len);
                    }
                    break;
                    
                case 4: // 经度方向 (E/W)
                    if (field_len > 0 && *field_start == 'W') {
                        gpsInfo.longitude = -gpsInfo.longitude;
                    }
                    break;
                    
                case 5: // GPS质量指示
                    gpsInfo.quality = (*field_start - '0');
                    break;
                    
                case 6: // 卫星数量
                    gpsInfo.satellites = atoi(field_start);
                    break;
                    
                case 7: // HDOP
                    gpsInfo.hdop = atof(field_start);
                    break;
                    
                case 8: // 海拔高度
                    gpsInfo.altitude = atof(field_start);
                    break;
                    
                case 10: // 大地水准面高度
                    gpsInfo.geoid_height = atof(field_start);
                    break;
                    
                case 12: // DGPS数据年龄
                    gpsInfo.dgps_age = atof(field_start);
                    break;
                    
                case 13: // DGPS参考站ID
                    gpsInfo.dgps_station = atoi(field_start);
                    break;
            }
            
            field_index++;
            if (*ptr == '*') break; // 遇到校验和结束
            ptr++; // 跳过逗号
            field_start = ptr;
        } else {
            ptr++;
        }
    }
    
    // 设置GPS有效标志
    gpsInfo.isValid = (gpsInfo.quality > 0 && gpsInfo.satellites >= 4);
    gpsInfo.isVaild = gpsInfo.isValid; // 兼容旧字段名
}

// 快速解析坐标 - 避免String操作
double parseCoordinateFast(const char* coord, int len)
{
    if (len < 4) return 0.0;
    
    // 找到小数点位置
    const char* dot = strchr(coord, '.');
    if (!dot) return 0.0;
    
    int dot_pos = dot - coord;
    if (dot_pos < 2) return 0.0;
    
    // 提取度数和分钟
    char degrees_str[8] = {0};
    char minutes_str[16] = {0};
    
    // 复制度数部分
    int degrees_len = dot_pos - 2;
    if (degrees_len > 0 && degrees_len < sizeof(degrees_str)) {
        strncpy(degrees_str, coord, degrees_len);
        degrees_str[degrees_len] = '\0';
    }
    
    // 复制分钟部分
    int minutes_len = len - dot_pos + 2;
    if (minutes_len > 0 && minutes_len < sizeof(minutes_str)) {
        strncpy(minutes_str, coord + dot_pos - 2, minutes_len);
        minutes_str[minutes_len] = '\0';
    }
    
    double degrees = atof(degrees_str);
    double minutes = atof(minutes_str);
    
    // 转换为十进制度数
    return degrees + (minutes / 60.0);
}

// 使用示例
void processUart1Message_Fast(SEMP_PARSE_STATE *parse, uint16_t type)
{
    const char* nmea = (const char*)parse->buffer;
    
    if (strstr(nmea, "GNGGA")) {
        parseGNGGA_Fast(nmea);
    }
    else if (strstr(nmea, "GPTXT")) {
        // 处理固件版本信息
        const char* v_start = strstr(nmea, "V");
        const char* v_end = strstr(nmea, "*");
        if (v_start && v_end && v_end > v_start) {
            int len = v_end - v_start;
            if (len < sizeof(gpsInfo.firmwareVersion)) {
                strncpy(gpsInfo.firmwareVersion, v_start, len);
                gpsInfo.firmwareVersion[len] = '\0';
            }
        }
    }
}
