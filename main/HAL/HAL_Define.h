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
}present_devices;

typedef struct online_devices
{
    bool microSd = false;
}online_devices;

typedef struct settings
{
    bool enableSd = false;
    uint8_t SdDivrerFrequency = 16; //Mhz
}settings;

/*HAL_Sd*/
#define SD_FAT_TYPE 3
extern SemaphoreHandle_t sdCardSemaphore;
extern SdCardStatus SdStatus;
/*HAL*/
extern present_devices present;
extern online_devices online;
extern settings systemSettings;