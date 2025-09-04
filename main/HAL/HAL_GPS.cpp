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
    if(nema.indexOf("GPTXT") != -1)
    {
        strcpy(gpsInfo.firmwareVersion, nema.substring(nema.indexOf("V"), nema.indexOf("*")).c_str());
    }
    // else if(nema.indexOf("GNGGA") != -1)
    // {
    //     printf("%s\n", nema.c_str());
    // }
    // else if(nema.indexOf("GNGSA") != -1)
    // {
    //     printf("%s\n", nema.c_str());
    // }
}


void HAL::GPS_Init()
{
    // Initialize the main parser
    rtkParse = sempBeginParser(rtkParserTable, rtkParserCount, rtkParserNames,
                               rtkParserNameCount,
                               0,                   // Scratchpad bytes
                               1024 * 6,            // Buffer length
                               processUart1Message, // eom Call Back
                               "rtkParse"         // Parser Name
                               );
    if (!rtkParse)
    {
        log_e("Failed to initialize the RTK parser");
        return;
    }

    if (gnssSerial == nullptr)
        gnssSerial = new HardwareSerial(1);

    gnssSerial->begin(9600, SERIAL_8N1, CONFIG_GPS_RX_PIN, CONFIG_GPS_TX_PIN);
    gnssSerial->setTimeout(2);
    gnssSerial->println("$PCAS01,5*19"); // Set the baud rate to 115200
    delay(100);
    gnssSerial->updateBaudRate(115200);
    gnssSerial->println("$PCAS03,0,0,0,0,0,0,0,0,0,0,,,0,0*02"); // Disable all messages
    gnssSerial->println("$PCAS06,0*1B"); // query for firmware version
    delay(100);
    while (gnssSerial->available())
    {
        uint8_t buffer[128];
        int bytesIncoming = gnssSerial->readBytes(buffer, sizeof(buffer));
        for (int x = 0; x < bytesIncoming; x++)
        {
            sempParseNextByte(rtkParse, buffer[x]);
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