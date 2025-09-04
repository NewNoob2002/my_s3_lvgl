#include "HAL_internal.h"
#include <SdFat.h>
#include "sdios.h"

ArduinoOutStream cout(Serial);

#if SD_FAT_TYPE == 0
SdFat sd;
File file;
#elif SD_FAT_TYPE == 1
SdFat32 sd;
File32 file;
#elif SD_FAT_TYPE == 2
SdExFat sd;
ExFile file;
#elif SD_FAT_TYPE == 3
SdFs *sdCard = nullptr;
FsFile *logFile = nullptr;
#else // SD_FAT_TYPE
#error Invalid SD_FAT_TYPE
#endif // SD_FAT_TYPE

SdCardStatus SdStatus;
// Mark when we start this specific log file so we can close it after x minutes
// and start a new one System crashes if two tasks access a file at the same
// time So we use a semaphore to see if the file system is available
SemaphoreHandle_t sdCardSemaphore = nullptr;

// #define SD_FORMAT
#if defined(SD_FORMAT)
// SdCardFactory constructs and initializes the appropriate card.
SdCardFactory cardFactory;
// Pointer to generic SD card.
SdCard *m_card = nullptr;
uint32_t const ERASE_SIZE = 262144L;

uint32_t cardSectorCount = 0;
uint8_t sectorBuffer[512] __attribute__((aligned(4)));

#define sdError(msg)                        \
  {                                         \
    cout << F("error: ") << F(msg) << endl; \
    sdErrorHalt();                          \
  }

void sdErrorHalt()
{
  if (!m_card)
  {
    cout << F("Invalid SD_CONFIG") << endl;
  }
  else if (m_card->errorCode())
  {
    if (m_card->errorCode() == SD_CARD_ERROR_CMD0)
    {
      cout << F("No card, wrong chip select pin, or wiring error?") << endl;
    }
    cout << F("SD errorCode: ") << hex << showbase;
    printSdErrorSymbol(&Serial, m_card->errorCode());
    cout << F(" = ") << int(m_card->errorCode()) << endl;
    cout << F("SD errorData = ") << int(m_card->errorData()) << endl;
  }
}

void formatCard()
{
  uint32_t firstBlock = 0;
  uint32_t lastBlock;
  uint16_t n = 0;
  // Select and initialize proper card driver.
  m_card = cardFactory.newCard(SdSpiConfig(CONFIG_SD_CS_PIN, DEDICATED_SPI, SD_SCK_MHZ(16), &CONFIG_SD_SPI));
  if (!m_card || m_card->errorCode())
  {
    sdError("card init failed.");
    return;
  }
  cardSectorCount = m_card->sectorCount();
  if (!cardSectorCount)
  {
    sdError("Get sector count failed.");
    return;
  }
  auto cardSize = cardSectorCount * 512e-9;
  printf("Card size: %f GB (GB = 1E9 bytes)\n", cardSize);
  printf("Card will be formated ");
  if (cardSectorCount > 67108864)
  {
    printf("exFAT\n");
  }
  else if (cardSectorCount > 4194304)
  {
    printf("FAT32\n");
  }
  else
  {
    printf("FAT16\n");
  }
  do
  {
    lastBlock = firstBlock + ERASE_SIZE - 1;
    if (lastBlock >= cardSectorCount)
    {
      lastBlock = cardSectorCount - 1;
    }
    if (!m_card->erase(firstBlock, lastBlock))
    {
      sdError("erase failed");
    }
    printf(".");
    if ((n++) % 64 == 63)
    {
      printf("\n");
    }
    firstBlock += ERASE_SIZE;
  } while (firstBlock < cardSectorCount);

  if (!m_card->readSector(0, sectorBuffer))
  {
    sdError("readBlock");
  }
  printf("All data set to %d\n", int(sectorBuffer[0]));
  printf("Erase done\n");
  ExFatFormatter exFatFormatter;
  FatFormatter fatFormatter;

  // Format exFAT if larger than 32GB.
  bool rtn = cardSectorCount > 67108864
                 ? exFatFormatter.format(m_card, sectorBuffer, &Serial)
                 : fatFormatter.format(m_card, sectorBuffer, &Serial);

  if (!rtn)
  {
    sdErrorHalt();
  }
  cout << F("Run the SdInfo example for format details.") << endl;
}

#endif // SD_FORMAT

void markSemaphore(SemaphoreFunction functionNumber)
{
  SdStatus.semaphoreFunction = functionNumber;
}

// Resolves the holder to a printable string
void getSemaphoreFunction(char *functionName)
{
  switch (SdStatus.semaphoreFunction)
  {
  default:
    strcpy(functionName, "Unknown");
    break;

  case FUNCTION_SYNC:
    strcpy(functionName, "Sync");
    break;
  case FUNCTION_WRITESD:
    strcpy(functionName, "Write");
    break;
  case FUNCTION_FILESIZE:
    strcpy(functionName, "FileSize");
    break;
  case FUNCTION_EVENT:
    strcpy(functionName, "Event");
    break;
  case FUNCTION_BEGINSD:
    strcpy(functionName, "BeginSD");
    break;
  case FUNCTION_REMOVEFILE:
    strcpy(functionName, "Remove file");
    break;
  case FUNCTION_RECORDLINE:
    strcpy(functionName, "Record Line");
    break;
  case FUNCTION_CREATEFILE:
    strcpy(functionName, "Create File");
    break;
  case FUNCTION_ENDLOGGING:
    strcpy(functionName, "End Logging");
    break;
  case FUNCTION_FINDLOG:
    strcpy(functionName, "Find Log");
    break;
  case FUNCTION_FILELIST:
    strcpy(functionName, "File List");
    break;
  }
}

void beginSdCard()
{
  if (present.microSd == false)
    return;
  else
    systemSettings.enableSd = true;

  while (systemSettings.enableSd)
  {
    if (sdCardSemaphore == nullptr)
      sdCardSemaphore = xSemaphoreCreateMutex();
    else if (xSemaphoreTake(sdCardSemaphore, 10) != pdPASS)
    {
      // This is OK since a retry will occur next loop
      log_d("sdCardSemaphore failed to yield, HAL_Sd.cpp line %d", __LINE__);
      break;
    }
    markSemaphore(FUNCTION_BEGINSD);

    if (sdCard == nullptr)
    {
      sdCard = new SdFat();
      if (!sdCard)
      {
        log_e("Failed to allocate the SdFat structure!");
        break;
      }
    }

    if (!sdCard->begin(SdSpiConfig(CONFIG_SD_CS_PIN, DEDICATED_SPI, SD_SCK_MHZ(systemSettings.SdDivrerFrequency), &CONFIG_SD_SPI)))
    {
      int tries = 0;
      int maxTries = 2;
      for (; tries < maxTries; tries++)
      {
        log_e("SD init failed - using SPI and SdFat. Trying again %d out of %d",
              tries + 1, maxTries);

        delay(250); // Give SD more time to power up, then try again
        if (sdCard->begin(SdSpiConfig(CONFIG_SD_CS_PIN, DEDICATED_SPI, SD_SCK_MHZ(systemSettings.SdDivrerFrequency), &CONFIG_SD_SPI)))
          break;
      }

      if (tries == maxTries)
      {
        log_e("SD init failed - using SPI and SdFat. Is card formatted?");
        // Check reset count and prevent rolling reboot
        // if (settings.resetCount < 5) {
        //   if (settings.forceResetOnSDFail == true)
        //     ESP.restart();
        // }
        break;
      }
    }

    SdStatus.isInitialized = true;
    csd_t csd;
    sdCard->card()->readCSD(&csd); // Card Specific Data
#if defined(CHECK_CARD_TYPE)
    cout << F("\nCard type: ");

    switch (sdCard->card()->type())
    {
    case SD_CARD_TYPE_SD1:
      cout << F("SD1\n");
      break;

    case SD_CARD_TYPE_SD2:
      cout << F("SD2\n");
      break;

    case SD_CARD_TYPE_SDHC:
      if (csd.capacity() < 70000000)
      {
        cout << F("SDHC\n");
      }
      else
      {
        cout << F("SDXC\n");
      }
      break;

    default:
      cout << F("Unknown\n");
    }
#endif
    // uint32_t eraseSize = csd.eraseSize();
    // cout << F("\ncardSize: ") << 0.000512 * csd.capacity();
    // cout << F(" MB (MB = 1,000,000 bytes)\n");

    // cout << F("flashEraseSize: ") << int(eraseSize) << F(" blocks\n");
    // cout << F("eraseSingleBlock: ");
    // if (csd.eraseSingleBlock())
    // {
    //   cout << F("true\n");
    // }
    // else
    // {
    //   cout << F("false\n");
    // }

    if (!sdCard->volumeBegin())
    {
      cout << F("\nvolumeBegin failed. Is the card formatted?\n");
      return;
    }
    SdStatus.sdCardSizeMb = 0.000512 * csd.capacity();
    SdStatus.sdFreeSpaceMb = sdCard->freeClusterCount() * (sdCard->bytesPerCluster()/1024)/1024;
    if(SdStatus.sdFreeSpaceMb <= 100)
      SdStatus.outOfSDSpace = true;
    else
      SdStatus.outOfSDSpace = false;

    online.microSd = true;
    break;
  }
}

bool HAL::SD_Init()
{
  if (present.microSd)
  {
    if (!SPI.begin(CONFIG_SD_SCK_PIN, CONFIG_SD_MISO_PIN, CONFIG_SD_MOSI_PIN))
    {
      log_e("\nSPI.begin() failed!");
      return false;
    }
    // formatCard();
    beginSdCard();
  }
  return true;
}