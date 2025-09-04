#include "HAL_internal.h"
#include "X-GNSS.h"
#include "SparkFun_Extensible_Message_Parser.h"

using namespace HAL;

HardwareSerial *gnssSerial = nullptr;
SEMP_PARSE_STATE *rtkParse = nullptr;
X_GNSS *gnss = nullptr;
static GPS_Info_t gpsInfo;
// List the parsers to be included
SEMP_PARSE_ROUTINE const rtkParserTable[] = {
    sempNmeaPreamble,
};
const int rtkParserCount = sizeof(rtkParserTable) / sizeof(rtkParserTable[0]);

// List the names of the parsers
const char *const rtkParserNames[] = {
    "NMEA",
};
const int rtkParserNameCount = sizeof(rtkParserNames) / sizeof(rtkParserNames[0]);

void GPS_HotStart()
{
    gnssSerial->println("$PCAS10,0*1C");
}

void GPS_WarmStart()
{
    gnssSerial->println("$PCAS10,1*1D");
}

void GPS_ColdStart()
{
    gnssSerial->println("$PCAS10,2*1E");
}

void GPS_Freset()
{
    gnssSerial->println("$PCAS10,3*1F");
}

// 解析坐标函数 (DDMM.mmmm 或 DDDMM.mmmm 格式)
double parseCoordinate(const String& coord)
{
    if (coord.length() < 4) return 0.0;
    
    // 找到小数点位置
    int dot_pos = coord.indexOf('.');
    if (dot_pos == -1) return 0.0;
    
    // 提取度数和分钟
    String degrees_str = coord.substring(0, dot_pos - 2);
    String minutes_str = coord.substring(dot_pos - 2);
    
    double degrees = degrees_str.toDouble();
    double minutes = minutes_str.toDouble();
    
    // 转换为十进制度数
    return degrees + (minutes / 60.0);
}
// GNGGA解析函数
void parseGNGGA(const String& nmea)
{
    // GNGGA格式: $GNGGA,time,lat,lat_dir,lon,lon_dir,quality,num_sv,hdop,alt,alt_unit,sep,sep_unit,diff_age,diff_station*checksum
    // 示例: $GNGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47
    
    int comma_count = 0;
    int start_pos = 0;
    int end_pos = 0;
    
    // 跳过$GNGGA,
    start_pos = nmea.indexOf(',') + 1;
    
    while (start_pos < nmea.length() && comma_count < 15) {
        end_pos = nmea.indexOf(',', start_pos);
        if (end_pos == -1) {
            end_pos = nmea.indexOf('*', start_pos);
            if (end_pos == -1) end_pos = nmea.length();
        }
        
        String field = nmea.substring(start_pos, end_pos);
        
        switch (comma_count) {
            case 0: // UTC时间 (HHMMSS.sss)
                if (field.length() >= 6) {
                    gpsInfo.clock.hour = field.substring(0, 2).toInt();
                    gpsInfo.clock.minute = field.substring(2, 4).toInt();
                    gpsInfo.clock.second = field.substring(4, 6).toInt();
                }
                break;
                
            case 1: // 纬度 (DDMM.mmmm)
                if (field.length() > 0) {
                    gpsInfo.latitude = parseCoordinate(field);
                }
                break;
                
            case 2: // 纬度方向 (N/S)
                if (field == "S") {
                    gpsInfo.latitude = -gpsInfo.latitude;
                }
                break;
                
            case 3: // 经度 (DDDMM.mmmm)
                if (field.length() > 0) {
                    gpsInfo.longitude = parseCoordinate(field);
                }
                break;
                
            case 4: // 经度方向 (E/W)
                if (field == "W") {
                    gpsInfo.longitude = -gpsInfo.longitude;
                }
                break;
                
            case 5: // GPS质量指示 (0=无效, 1=GPS, 2=DGPS)
                gpsInfo.quality = field.toInt();
                break;
                
            case 6: // 使用的卫星数量
                gpsInfo.satellites = field.toInt();
                break;
                
            case 7: // 水平精度因子
                gpsInfo.hdop = field.toFloat();
                break;
                
            case 8: // 海拔高度
                gpsInfo.altitude = field.toFloat();
                break;
                
            case 9: // 海拔单位 (M=米)
                // 通常为M，可以忽略
                break;
                
            case 10: // 大地水准面高度
                gpsInfo.geoid_height = field.toFloat();
                break;
                
            case 11: // 大地水准面单位
                // 通常为M，可以忽略
                break;
                
            case 12: // DGPS数据年龄
                gpsInfo.dgps_age = field.toFloat();
                break;
                
            case 13: // DGPS参考站ID
                gpsInfo.dgps_station = field.toInt();
                break;
        }
        
        comma_count++;
        start_pos = end_pos + 1;
    }
    
    // 设置GPS有效标志
    gpsInfo.isValid = (gpsInfo.quality > 0 && gpsInfo.satellites >= 4);
    
    gnss->commitAll();
}

void processUart1Message(SEMP_PARSE_STATE *parse, uint16_t type)
{
    String nema = String((char *)parse->buffer);
    if (nema.indexOf("GPTXT") != -1)
    {
        strcpy(gpsInfo.firmwareVersion, nema.substring(nema.indexOf("V"), nema.indexOf("*")).c_str());
    }
    else if (nema.indexOf("GNGGA") != -1)
    {
        parseGNGGA(nema);
    }
    else if (nema.indexOf("GNGSA") != -1)
    {
        printf("%s\n", nema.c_str());
    }
}

void HAL::GPS_Init()
{
    memset(&gpsInfo, 0, sizeof(gpsInfo));
    // Initialize the main parser
    rtkParse = sempBeginParser(rtkParserTable, rtkParserCount, rtkParserNames,
                               rtkParserNameCount,
                               0,                   // Scratchpad bytes
                               1024 * 6,            // Buffer length
                               processUart1Message, // eom Call Back
                               "rtkParse"           // Parser Name
    );
    if (!rtkParse)
    {
        log_e("Failed to initialize the RTK parser");
        return;
    }
    if (gnss == nullptr)
    {
        gnss = new X_GNSS();
        gnss->begin(rtkParse);
    }

    if (gnssSerial == nullptr)
        gnssSerial = new HardwareSerial(1);

    gnssSerial->begin(9600, SERIAL_8N1, CONFIG_GPS_RX_PIN, CONFIG_GPS_TX_PIN);
    gnssSerial->setTimeout(2);
    gnssSerial->println("$PCAS01,5*19"); // Set the baud rate to 115200
    delay(100);
    gnssSerial->updateBaudRate(115200);
    gnssSerial->println("$PCAS03,0,0,0,0,0,0,0,0,0,0,,,0,0*02"); // Disable all messages
    gnssSerial->println("$PCAS06,0*1B");                         // query for firmware version
    delay(100);
    while (gnssSerial->available())
    {
        uint8_t buffer[128];
        int bytesIncoming = gnssSerial->readBytes(buffer, sizeof(buffer));
        for (int x = 0; x < bytesIncoming; x++)
        {
            gnss->decode(buffer[x]);
        }
    }
    gnssSerial->println("$PCAS03,1,0,3,2,1,0,1,0,0,0,,,1,1*02"); // GPGGA 1, GPGSA 3, GPGSV 2, GPRMC 1, GPZDA 1
}

void HAL::GPS_Update(void *e)
{
    while (true)
    {
        if (!rtkParse)
        {
            log_e("Failed to initialize the RTK parser");
            return;
        }
        if (gnssSerial->available())
        {
            uint8_t GPRS_RX_BUFF[512];
            int bytesIncoming = gnssSerial->readBytes(GPRS_RX_BUFF, sizeof(GPRS_RX_BUFF));

            for (int x = 0; x < bytesIncoming; x++)
            {
                gnss->decode(GPRS_RX_BUFF[x]);
            }
        }

        delay(1);
        taskYIELD();
    }
    // Done parsing incoming data, free the parse buffer
    sempStopParser(&rtkParse);

    // Stop notification
    vTaskDelete(NULL);
}

void HAL::GPS_GetInfo(GPS_Info_t *info)
{
    info->satellites = gpsInfo.satellites;
    info->isVaild = gpsInfo.isVaild;
    info->clock = gpsInfo.clock;
    info->longitude = gpsInfo.longitude;
    info->latitude = gpsInfo.latitude;
    info->altitude = gpsInfo.altitude;
    info->course = gpsInfo.course;
    info->speed = gpsInfo.speed;
    memcpy(info->firmwareVersion, gpsInfo.firmwareVersion, sizeof(gpsInfo.firmwareVersion));
}