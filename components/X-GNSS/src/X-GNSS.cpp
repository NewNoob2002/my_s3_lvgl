#include "X-GNSS.h"

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>

#define _RMCterm "RMC"
#define _GGAterm "GGA"

X_GNSS::X_GNSS() : curSentenceType(GPS_SENTENCE_OTHER), curTermNumber(0), sentenceHasFix(false)
{
  memset(firmwareVersion, 0, sizeof(firmwareVersion));
}

void X_GNSS::decode(char c)
{
  if (rtkParse)
    sempParseNextByte(rtkParse, c);
}

void X_GNSS::nemaHandler(uint8_t *response, uint16_t length)
{
  this->curSentenceType = GPS_SENTENCE_OTHER;

  response[length] = '\0'; // Force terminator because strncasestr does not exist
  if (strcasestr((char *)response, "RMC") != NULL)
    curSentenceType = GPS_SENTENCE_RMC;
  else if (strcasestr((char *)response, "GGA") != NULL)
    curSentenceType = GPS_SENTENCE_GGA;
  else
    curSentenceType = GPS_SENTENCE_OTHER;

  this->parseCompleteSentence((char *)response);
  switch (curSentenceType)
  {
  case GPS_SENTENCE_RMC:
    date.commit();
    time.commit();
    if (sentenceHasFix)
    {
      location.commit();
      speed.commit();
      course.commit();
    }
    break;
  case GPS_SENTENCE_GGA:
    time.commit();
    if (sentenceHasFix)
    {
      location.commit();
      altitude.commit();
    }
    satellites.commit();
    hdop.commit();
    break;
  }
}

void X_GNSS::parseCompleteSentence(const char *sentence)
{
  if (this->curSentenceType == GPS_SENTENCE_OTHER)
    return;

  const char *ptr = sentence;

  // 跳过句子类型标识符 (例如: $GNGGA,)
  while (*ptr && *ptr != ',')
    ptr++;
  if (*ptr == ',')
    ptr++; // 跳过逗号

  this->curTermNumber = 1;

  // 解析每个字段
  while (*ptr)
  {
    const char *start = ptr;

    // 找到下一个逗号或结束符
    while (*ptr && *ptr != ',' && *ptr != '*')
      ptr++;

    // 创建字段字符串
    char term[32] = {0};
    int len = ptr - start;
    if (len > 0 && len < sizeof(term))
    {
      strncpy(term, start, len);
      this->parseTerm(term);
    }

    this->curTermNumber++; // 字段编号递增

    if (*ptr == ',' || *ptr == '*')
      ptr++;
    if (*ptr == '*')
      break; // 遇到校验和，停止解析
  }
}

void X_GNSS::parseTerm(const char *term)
{
  if (this->curSentenceType != GPS_SENTENCE_OTHER && term[0])
  {
    switch (COMBINE(this->curSentenceType, this->curTermNumber))
    {
    case COMBINE(GPS_SENTENCE_RMC, 1): // Time in both sentences
    case COMBINE(GPS_SENTENCE_GGA, 1):
      time.setTime(term);
      break;
    case COMBINE(GPS_SENTENCE_RMC, 2): // RMC validity
      sentenceHasFix = term[0] == 'A';
      break;
    case COMBINE(GPS_SENTENCE_RMC, 3): // Latitude
    case COMBINE(GPS_SENTENCE_GGA, 2):
      location.setLatitude(term);
      break;
    case COMBINE(GPS_SENTENCE_RMC, 4): // N/S
    case COMBINE(GPS_SENTENCE_GGA, 3):
      location.rawNewLatData.negative = term[0] == 'S';
      break;
    case COMBINE(GPS_SENTENCE_RMC, 5): // Longitude
    case COMBINE(GPS_SENTENCE_GGA, 4):
      location.setLongitude(term);
      break;
    case COMBINE(GPS_SENTENCE_RMC, 6): // E/W
    case COMBINE(GPS_SENTENCE_GGA, 5):
      location.rawNewLngData.negative = term[0] == 'W';
      break;
    case COMBINE(GPS_SENTENCE_RMC, 7): // Speed (RMC)
      speed.set(term);
      break;
    case COMBINE(GPS_SENTENCE_RMC, 8): // Course (RMC)
      course.set(term);
      break;
    case COMBINE(GPS_SENTENCE_RMC, 9): // Date (RMC)
      date.setDate(term);
      break;
    case COMBINE(GPS_SENTENCE_GGA, 6): // Fix data (GGA)
      sentenceHasFix = term[0] > '0';
      location.newFixQuality = (TinyGPSLocation::Quality)term[0];
      break;
    case COMBINE(GPS_SENTENCE_GGA, 7): // Satellites used (GGA)
      satellites.set(term);
      break;
    case COMBINE(GPS_SENTENCE_GGA, 8): // HDOP
      hdop.set(term);
      break;
    case COMBINE(GPS_SENTENCE_GGA, 9): // Altitude (GGA)
      altitude.set(term);
      break;
    case COMBINE(GPS_SENTENCE_RMC, 12):
      location.newFixMode = (TinyGPSLocation::Mode)term[0];
      break;
    }
  }
}

double X_GNSS::distanceBetween(double lat1, double long1, double lat2, double long2)
{
  // returns distance in meters between two positions, both specified
  // as signed decimal-degrees latitude and longitude. Uses great-circle
  // distance computation for hypothetical sphere of radius 6371009 meters.
  // Because Earth is no exact sphere, rounding errors may be up to 0.5%.
  // Courtesy of Maarten Lamers
  double delta = radians(long1 - long2);
  double sdlong = sin(delta);
  double cdlong = cos(delta);
  lat1 = radians(lat1);
  lat2 = radians(lat2);
  double slat1 = sin(lat1);
  double clat1 = cos(lat1);
  double slat2 = sin(lat2);
  double clat2 = cos(lat2);
  delta = (clat1 * slat2) - (slat1 * clat2 * cdlong);
  delta = sq(delta);
  delta += sq(clat2 * sdlong);
  delta = sqrt(delta);
  double denom = (slat1 * slat2) + (clat1 * clat2 * cdlong);
  delta = atan2(delta, denom);
  return delta * _GPS_EARTH_MEAN_RADIUS;
}

double X_GNSS::courseTo(double lat1, double long1, double lat2, double long2)
{
  // returns course in degrees (North=0, West=270) from position 1 to position 2,
  // both specified as signed decimal-degrees latitude and longitude.
  // Because Earth is no exact sphere, calculated course may be off by a tiny fraction.
  // Courtesy of Maarten Lamers
  double dlon = radians(long2 - long1);
  lat1 = radians(lat1);
  lat2 = radians(lat2);
  double a1 = sin(dlon) * cos(lat2);
  double a2 = sin(lat1) * cos(lat2) * cos(dlon);
  a2 = cos(lat1) * sin(lat2) - a2;
  a2 = atan2(a1, a2);
  if (a2 < 0.0)
  {
    a2 += TWO_PI;
  }
  return degrees(a2);
}

const char *X_GNSS::cardinal(double course)
{
  static const char *directions[] = {"N", "NNE", "NE", "ENE", "E", "ESE", "SE", "SSE", "S", "SSW", "SW", "WSW", "W", "WNW", "NW", "NNW"};
  int direction = (int)((course + 11.25f) / 22.5f);
  return directions[direction % 16];
}

int32_t X_GNSS::parseDecimal(const char *term)
{
  bool negative = *term == '-';
  if (negative)
    ++term;
  int32_t ret = 100 * (int32_t)atol(term);
  while (isdigit(*term))
    ++term;
  if (*term == '.' && isdigit(term[1]))
  {
    ret += 10 * (term[1] - '0');
    if (isdigit(term[2]))
      ret += term[2] - '0';
  }
  return negative ? -ret : ret;
}

// static
// Parse degrees in that funny NMEA format DDMM.MMMM
void X_GNSS::parseDegrees(const char *term, RawDegrees &deg)
{
  uint32_t leftOfDecimal = (uint32_t)atol(term);
  uint16_t minutes = (uint16_t)(leftOfDecimal % 100);
  uint32_t multiplier = 10000000UL;
  uint32_t tenMillionthsOfMinutes = minutes * multiplier;

  deg.deg = (int16_t)(leftOfDecimal / 100);

  while (isdigit(*term))
    ++term;

  if (*term == '.')
    while (isdigit(*++term))
    {
      multiplier /= 10;
      tenMillionthsOfMinutes += (*term - '0') * multiplier;
    }

  deg.billionths = (5 * tenMillionthsOfMinutes + 1) / 3;
  deg.negative = false;
}

// TinyGPSLocation 方法的实现
void TinyGPSLocation::setLatitude(const char *term)
{
  X_GNSS::parseDegrees(term, rawNewLatData);
}

void TinyGPSLocation::setLongitude(const char *term)
{
  X_GNSS::parseDegrees(term, rawNewLngData);
}
