#include "HAL_internal.h"
#include "SparkFun_Extensible_Message_Parser.h"

using namespace HAL;

HardwareSerial *gnssSerial = nullptr;
SEMP_PARSE_STATE *rtkParse = nullptr;
GPS_Info_t gpsInfo;
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

void processUart1Message(SEMP_PARSE_STATE *parse, uint16_t type)
{
    char *responsePointer = strstr((char *)parse->buffer, "GNGGA");
    static uint8_t field_count = 0;
    if (responsePointer != nullptr) // Found
    {
        char *ptr = (char*)parse->buffer;
        while (*ptr)
        {
            char *field_start = ptr;

            // 查找字段结束位置
            while (*ptr && *ptr != ',' && *ptr != '*')
                ptr++;

            int field_len = ptr - field_start;
            field_count++;
            char buf[32];
            // 提取目标字段
            switch (field_count)
            {
            case 2: // UTC 时间 (HHMMSS.sss)
                strncpy(buf, field_start, field_len);
                break;

            case 3: // 纬度 (DDMM.mmmm)
                // strncpy(latitude, field_start, field_len < 15 ? field_len : 15);
                // latitude[field_len < 15 ? field_len : 15] = '\0';
                // gpsInfo.latitude = atof(latitude);
                break;

            case 4: // 纬度方向 (N/S)
                if (field_len > 0)
                    // lat_dir = *field_start;
                break;

            case 5: // 经度 (DDDMM.mmmm)
                // strncpy(longitude, field_start, field_len < 15 ? field_len : 15);
                // longitude[field_len < 15 ? field_len : 15] = '\0';
                break;

            case 6: // 经度方向 (E/W)
                // if (field_len > 0)
                //     lon_dir = *field_start;
                break;

            case 8: // 卫星数量
                // numSv = atoi(field_start);
                // // 提前终止（已获取所有目标字段）
                // ptr = strchr(ptr, '\0'); // 跳到字符串结尾
                break;
            }

            if (*ptr == ',')
                ptr++; // 跳过逗号
            if (*ptr == '*')
                break; // 遇到校验和结束
        }

        // Reset field count for next message
        field_count = 0;
    }
}
void HAL::GPS_Init()
{
    // Initialize the main parser
    rtkParse = sempBeginParser(rtkParserTable, rtkParserCount, rtkParserNames,
                               rtkParserNameCount,
                               0,                   // Scratchpad bytes
                               1024 * 6,            // Buffer length
                               processUart1Message, // eom Call Back
                               "rtkParse");         // Parser Name
    if (!rtkParse)
    {
        log_e("Failed to initialize the RTK parser");
        return;
    }

    if (gnssSerial == nullptr)
        gnssSerial = new HardwareSerial(1);

    gnssSerial->begin(9600, SERIAL_8N1, CONFIG_GPS_RX_PIN, CONFIG_GPS_TX_PIN);
    gnssSerial->setTimeout(2);
}

void HAL::GPS_Update(void *e)
{
    while (true)
    {
        if (gnssSerial->available())
        {
            uint8_t GPRS_RX_BUFF[512];
            int bytesIncoming = gnssSerial->readBytes(GPRS_RX_BUFF, sizeof(GPRS_RX_BUFF));

            // systemWrite(GPRS_RX_BUFF, bytesIncoming);
            for (int x = 0; x < bytesIncoming; x++)
            {
                sempParseNextByte(rtkParse, GPRS_RX_BUFF[x]);
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
    
}