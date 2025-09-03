#pragma once


/*=========================
   Hardware Configuration
 *=========================*/
#ifndef PIN_NOT_DEFINED
#define PIN_NOT_DEFINED -1
#endif // PIN_NOT_DEFINED

/* Screen */
#define CONFIG_SCREEN_CS_PIN        10
#define CONFIG_SCREEN_DC_PIN        11
#define CONFIG_SCREEN_RST_PIN       1
#define CONFIG_SCREEN_SCK_PIN       12
#define CONFIG_SCREEN_MOSI_PIN      13
#define CONFIG_SCREEN_BLK_PIN       14
#define CONFIG_SCREEN_SPI_NUM       2
#define CONFIG_SCREEN_SPI_SHARED    false
#define CONFIG_SCREEN_ROTATION      1 //

#define CONFIG_SCREEN_HOR_RES       172
#define CONFIG_SCREEN_VER_RES       320

#define CONFIG_SCREEN_OFFSETX1    (240 - CONFIG_SCREEN_HOR_RES) / 2
#define CONFIG_SCREEN_OFFSETY1    (320 - CONFIG_SCREEN_VER_RES) / 2
#define CONFIG_SCREEN_OFFSETX2    (240 - CONFIG_SCREEN_HOR_RES) / 2
#define CONFIG_SCREEN_OFFSETY2    (320 - CONFIG_SCREEN_VER_RES) / 2
/* Battery */
// #define CONFIG_BAT_DET_PIN          PA1
// #define CONFIG_BAT_CHG_DET_PIN      PA11

/* Buzzer */
// #define CONFIG_BUZZ_PIN             PA0  // TIM2

/* GPS */
#define CONFIG_GPS_USE_TRANSPARENT  0
#define CONFIG_GPS_BUF_OVERLOAD_CHK 0
#define CONFIG_GPS_TX_PIN           5
#define CONFIG_GPS_RX_PIN           4

/* IMU */
// #define CONFIG_IMU_INT1_PIN         PB10
// #define CONFIG_IMU_INT2_PIN         PB11

/* I2C */
// #define CONFIG_MCU_SDA_PIN          PB7
// #define CONFIG_MCU_SDL_PIN          PB6

/* Encoder */
// #define CONFIG_ENCODER_B_PIN        PB5
// #define CONFIG_ENCODER_A_PIN        PB4
// #define CONFIG_ENCODER_PUSH_PIN     PB3

/* Power */
// #define CONFIG_POWER_EN_PIN         PA12
// #define CONFIG_POWER_WAIT_TIME      1000
// #define CONFIG_POWER_SHUTDOWM_DELAY 5000
// #define CONFIG_POWER_BATT_CHG_DET_PULLUP    true

/* Debug USART */
#define CONFIG_DEBUG_SERIAL         Serial

/* SD CARD */
#define CONFIG_SD_SPI               SPI
#define CONFIG_SD_MOSI_PIN          42
#define CONFIG_SD_MISO_PIN          40
#define CONFIG_SD_SCK_PIN           41
#define CONFIG_SD_CS_PIN            2