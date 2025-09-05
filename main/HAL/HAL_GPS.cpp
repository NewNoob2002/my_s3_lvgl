#include "HAL_internal.h"
#include "X-GNSS.h"
#include "SparkFun_Extensible_Message_Parser.h"

using namespace HAL;

HardwareSerial *gnssSerial = nullptr;
SEMP_PARSE_STATE *rtkParse = nullptr;
X_GNSS *gnss = nullptr;
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


void gps_dump_info(GPS_Info_t *info);

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

void processUart1Message(SEMP_PARSE_STATE *parse, uint16_t type)
{
    String nema = String((char *)parse->buffer);
    if (nema.indexOf("GPTXT") != -1)
    {
        strcpy(gnss->firmwareVersion, nema.substring(nema.indexOf("V"), nema.indexOf("*")).c_str());
    }
    else
    {
        gnss->nemaHandler(parse->buffer, parse->length);
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

bool HAL::GPS_GetInfo(GPS_Info_t *info)
{
    memset(info, 0, sizeof(GPS_Info_t));

    info->isVaild = gnss->location.isValid();
    info->longitude = gnss->location.lng();
    info->latitude = gnss->location.lat();
    info->altitude = gnss->altitude.meters();
    info->speed = gnss->speed.kmph();
    info->course = gnss->course.deg();

    info->clock.year = gnss->date.year();
    info->clock.month = gnss->date.month();
    info->clock.day = gnss->date.day();
    info->clock.hour = gnss->time.hour();
    info->clock.minute = gnss->time.minute();
    info->clock.second = gnss->time.second();
    info->satellites = gnss->satellites.value();
    strcpy(info->firmwareVersion, gnss->firmwareVersion);

    //gps_dump_info(info);
    return info->isVaild;
}

void gps_dump_info(GPS_Info_t *info)
{
    GPS_LOG_INFO("Valid: %s, Longitude: %f, Latitude: %f, Altitude: %f, Speed: %f, Course: %f, Satellites: %d, Firmware Version: %s, Year: %d, Month: %d, Day: %d, Hour: %d, Minute: %d, Second: %d", info->isVaild ? "true" : "false", info->longitude, info->latitude, info->altitude, info->speed, info->course, info->satellites, info->firmwareVersion, info->clock.year, info->clock.month, info->clock.day, info->clock.hour, info->clock.minute, info->clock.second);
}