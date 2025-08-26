#include "HAL_internal.h"
#include <SdFat.h>
#include "sdios.h"


#define SD_CONFIG SdSpiConfig(SS, DEDICATED_SPI, SD_SCK_MHZ(16), &SPI)

// SdCardFactory constructs and initializes the appropriate card.
SdCardFactory cardFactory;
// Pointer to generic SD card.
SdCard *m_card = nullptr;
uint32_t const ERASE_SIZE = 262144L;
ArduinoOutStream cout(Serial);
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
  m_card = cardFactory.newCard(SdSpiConfig(SS, DEDICATED_SPI, SD_SCK_MHZ(16)));
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