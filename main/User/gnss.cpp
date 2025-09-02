#include "gnss.h"
#include <Arduino_GFX_Library.h>

SEMP_PARSE_STATE *rtkParse = nullptr;
HardwareSerial *gnssSerial = nullptr;
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

extern Arduino_GFX *gfx;

int field_count = 0;
char utc_time[16] = {0};      // HHMMSS.sss 格式
char latitude[16] = {0};      // DDMM.mmmm 格式
char lat_dir = 0;             // N/S
char longitude[16] = {0};     // DDDMM.mmmm 格式
char lon_dir = 0;             // E/W
int numSv = 0;                // 卫星数量

void processUart1Message(SEMP_PARSE_STATE *parse, uint16_t type)
{
    char *responsePointer = strstr((char *)parse->buffer, "GNGGA");
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

            // 提取目标字段
            switch (field_count)
            {
            case 2: // UTC 时间 (HHMMSS.sss)
                strncpy(utc_time, field_start, field_len < 15 ? field_len : 15);
                utc_time[field_len < 15 ? field_len : 15] = '\0';
                break;

            case 3: // 纬度 (DDMM.mmmm)
                strncpy(latitude, field_start, field_len < 15 ? field_len : 15);
                latitude[field_len < 15 ? field_len : 15] = '\0';
                break;

            case 4: // 纬度方向 (N/S)
                if (field_len > 0)
                    lat_dir = *field_start;
                break;

            case 5: // 经度 (DDDMM.mmmm)
                strncpy(longitude, field_start, field_len < 15 ? field_len : 15);
                longitude[field_len < 15 ? field_len : 15] = '\0';
                break;

            case 6: // 经度方向 (E/W)
                if (field_len > 0)
                    lon_dir = *field_start;
                break;

            case 8: // 卫星数量
                numSv = atoi(field_start);
                // 提前终止（已获取所有目标字段）
                ptr = strchr(ptr, '\0'); // 跳到字符串结尾
                break;
            }

            if (*ptr == ',')
                ptr++; // 跳过逗号
            if (*ptr == '*')
                break; // 遇到校验和结束
        }

        gfx->fillScreen(BLACK);
        gfx->setCursor(0, 0);
        gfx->setTextColor(WHITE);
        gfx->printf("UTC Time: %s\n", utc_time);
        gfx->printf("Latitude: %s %c\n", latitude, lat_dir);
        gfx->printf("Longitude: %s %c\n", longitude, lon_dir);
        gfx->printf("Number of Satellites: %d\n", numSv);

        // Reset field count for next message
        field_count = 0;
    }
}

void gnssReadTask(void *e)
{
    // Initialize the main parser
    rtkParse = sempBeginParser(rtkParserTable, rtkParserCount, rtkParserNames,
                               rtkParserNameCount,
                               0,                   // Scratchpad bytes
                               1024 * 6,            // Buffer length
                               processUart1Message, // eom Call Back
                               "rtkParse");         // Parser Name

    if (!rtkParse)
        printf("Failed to initialize the RTK parser");

    if (gnssSerial == nullptr)
    {
        gnssSerial = new HardwareSerial(1);
    }
    gnssSerial->begin(9600, SERIAL_8N1, 18, 19);
    gnssSerial->setTimeout(2);
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

void gnssTaskBegin()
{
    xTaskCreatePinnedToCore(
        gnssReadTask, // Function to call
        "gnssRead",   // Just for humans
        3072,         // Stack Size
        nullptr,      // Task input parameter
        2,            // Priority
        nullptr,      // Task handle
        1);
}