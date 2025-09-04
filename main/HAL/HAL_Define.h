#pragma once
/**
 * \file
 * \brief HAL define include file.
 */
typedef enum
{
    FUNCTION_NONE = 0,
    FUNCTION_SYNC,
    FUNCTION_WRITESD,
    FUNCTION_FILESIZE,
    FUNCTION_EVENT,
    FUNCTION_BEGINSD,
    FUNCTION_STARTLOGGING,
    FUNCTION_REMOVEFILE,
    FUNCTION_RECORDLINE,
    FUNCTION_CREATEFILE,
    FUNCTION_ENDLOGGING,
    FUNCTION_FINDLOG,
    FUNCTION_FILELIST,
    FUNCTION_LVGL_OPEN,
    FUNCTION_LVGL_CLOSE,
    FUNCTION_LVGL_READ,
    FUNCTION_LVGL_WRITE,
    FUNCTION_LVGL_SEEK,
    FUNCTION_LVGL_TELL,
    FUNCTION_LVGL_DIR_OPEN,
    FUNCTION_LVGL_DIR_CLOSE,
} SemaphoreFunction;

typedef struct SdCardStatus
{
    uint32_t sdCardSizeMb = 0;
    uint32_t sdFreeSpaceMb = 0;
    bool outOfSDSpace = false;
    bool isInitialized = false;
    SemaphoreFunction semaphoreFunction = FUNCTION_NONE;
} SdCardStatus;

typedef struct present_device
{
    bool microSd = false;
    bool display = false;
} present_devices;

typedef struct online_devices
{
    bool microSd = false;
} online_devices;

typedef struct settings
{
    bool enableSd = false;
    uint8_t SdDivrerFrequency = 16; // Mhz
} settings;

namespace HAL
{
    /* Clock */
    typedef struct
    {
        uint16_t year;
        uint8_t month;
        uint8_t day;
        uint8_t week;
        uint8_t hour;
        uint8_t minute;
        uint8_t second;
        uint16_t millisecond;
    } Clock_Info_t;
    /* GPS */
    typedef struct
    {
        double longitude;
        double latitude;
        float altitude;
        float course;
        float speed;
        int16_t satellites;
        bool isVaild;
        Clock_Info_t clock;
        char firmwareVersion[16];
    } GPS_Info_t;
}

/*HAL_Sd*/
#define SD_FAT_TYPE 3
extern SemaphoreHandle_t sdCardSemaphore;
extern SdCardStatus SdStatus;
void markSemaphore(SemaphoreFunction functionNumber);
/*HAL_Display*/
extern SemaphoreHandle_t lvglSemaphore;

/*HAL*/
extern present_devices present;
extern online_devices online;
extern settings systemSettings;