#ifndef __X_GNSS_H
#define __X_GNSS_H
#include "Arduino.h"
#include "SparkFun_Extensible_Message_Parser.h"

#define _GPS_VERSION "1.0.0" // software version of this library
#define _GPS_MPH_PER_KNOT 1.15077945
#define _GPS_MPS_PER_KNOT 0.51444444
#define _GPS_KMPH_PER_KNOT 1.852
#define _GPS_MILES_PER_METER 0.00062137112
#define _GPS_KM_PER_METER 0.001
#define _GPS_FEET_PER_METER 3.2808399
#define _GPS_MAX_FIELD_SIZE 15
#define _GPS_EARTH_MEAN_RADIUS 6371009 // old: 6372795

struct RawDegrees
{
    uint16_t deg;
    uint32_t billionths;
    bool negative;

public:
    RawDegrees() : deg(0), billionths(0), negative(false)
    {
    }
};

struct TinyGPSLocation
{
    friend class X_GNSS;

public:
    enum Quality
    {
        Invalid = '0',
        GPS = '1',
        DGPS = '2',
        PPS = '3',
        RTK = '4',
        FloatRTK = '5',
        Estimated = '6',
        Manual = '7',
        Simulated = '8'
    };
    enum Mode
    {
        N = 'N',
        A = 'A',
        D = 'D',
        E = 'E'
    };

    bool isValid() const { return valid; }
    bool isUpdated() const { return updated; }
    uint32_t age() const { return valid ? millis() - lastCommitTime : (uint32_t)ULONG_MAX; }
    const RawDegrees &rawLat()
    {
        updated = false;
        return rawLatData;
    }
    const RawDegrees &rawLng()
    {
        updated = false;
        return rawLngData;
    }
    double lat()
    {
        updated = false;
        double ret = rawLatData.deg + rawLatData.billionths / 1000000000.0;
        return rawLatData.negative ? -ret : ret;
    }
    double lng()
    {
        updated = false;
        double ret = rawLngData.deg + rawLngData.billionths / 1000000000.0;
        return rawLngData.negative ? -ret : ret;
    }
    Quality FixQuality()
    {
        updated = false;
        return fixQuality;
    }
    Mode FixMode()
    {
        updated = false;
        return fixMode;
    }

    TinyGPSLocation() : valid(false), updated(false), fixQuality(Invalid), fixMode(N)
    {
    }

private:
    bool valid, updated;
    RawDegrees rawLatData, rawLngData, rawNewLatData, rawNewLngData;
    Quality fixQuality, newFixQuality;
    Mode fixMode, newFixMode;
    uint32_t lastCommitTime;
    void commit()
    {
        rawLatData = rawNewLatData;
        rawLngData = rawNewLngData;
        fixQuality = newFixQuality;
        fixMode = newFixMode;
        lastCommitTime = millis();
        valid = updated = true;
    }
    void setLatitude(const char *term, RawDegrees &deg)
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
    void setLongitude(const char *term, RawDegrees &deg)
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
};

struct TinyGPSDate
{
    friend class X_GNSS;

public:
    bool isValid() const { return valid; }
    bool isUpdated() const { return updated; }
    uint32_t age() const { return valid ? millis() - lastCommitTime : (uint32_t)ULONG_MAX; }

    uint32_t value()
    {
        updated = false;
        return date;
    }
    uint16_t year(){
        updated = false;
        uint16_t year = date % 100;
        return year + 2000;
    }
    uint8_t month(){
        updated = false;
        return (date / 100) % 100;
    }
    uint8_t day(){
        updated = false;
        return date / 10000;
    }

    TinyGPSDate() : valid(false), updated(false), date(0)
    {
    }

private:
    bool valid, updated;
    uint32_t date, newDate;
    uint32_t lastCommitTime;
    void commit()
    {
        date = newDate;
        lastCommitTime = millis();
        valid = updated = true;
    }
    void setDate(const char *term){
        newDate = atol(term);
    }
};

struct TinyGPSTime
{
    friend class X_GNSS;

public:
    bool isValid() const { return valid; }
    bool isUpdated() const { return updated; }
    uint32_t age() const { return valid ? millis() - lastCommitTime : (uint32_t)ULONG_MAX; }

    uint32_t value()
    {
        updated = false;
        return time;
    }
    uint8_t hour()
    {
        updated = false;
        return time / 10000;
    }
    uint8_t minute()
    {
        updated = false;
        return (time / 100) % 100;  
    }
    uint8_t second()
    {
        updated = false;
        return (time % 100);
    }

    TinyGPSTime() : valid(false), updated(false), time(0)
    {
    }

private:
    bool valid, updated;
    uint32_t time, newTime;
    uint32_t lastCommitTime;
    void commit()
    {
        time = newTime;
        lastCommitTime = millis();
        valid = updated = true;
    }
    void setTime(const char *term)
    {
        bool negative = *term == '-';
        if (negative) ++term;
        int32_t ret = (int32_t)atol(term);
        // while (isdigit(*term)) ++term;
        // if (*term == '.' && isdigit(term[1]))
        // {
        //   ret += 10 * (term[1] - '0');
        //   if (isdigit(term[2]))
        //     ret += term[2] - '0';
        // }
        newTime = negative ? -ret : ret;
    }
};

struct TinyGPSDecimal
{
    friend class X_GNSS;

public:
    bool isValid() const { return valid; }
    bool isUpdated() const { return updated; }
    uint32_t age() const { return valid ? millis() - lastCommitTime : (uint32_t)ULONG_MAX; }
    int32_t value()
    {
        updated = false;
        return val;
    }

    TinyGPSDecimal() : valid(false), updated(false), val(0)
    {
    }

private:
    bool valid, updated;
    uint32_t lastCommitTime;
    int32_t val, newval;
    void commit();
    void set(const char *term);
};

struct TinyGPSSpeed : TinyGPSDecimal
{
    double knots() { return value() / 100.0; }
    double mph() { return _GPS_MPH_PER_KNOT * value() / 100.0; }
    double mps() { return _GPS_MPS_PER_KNOT * value() / 100.0; }
    double kmph() { return _GPS_KMPH_PER_KNOT * value() / 100.0; }
};

struct TinyGPSCourse : public TinyGPSDecimal
{
    double deg() { return value() / 100.0; }
};

struct TinyGPSAltitude : TinyGPSDecimal
{
    double meters() { return value() / 100.0; }
    double miles() { return _GPS_MILES_PER_METER * value() / 100.0; }
    double kilometers() { return _GPS_KM_PER_METER * value() / 100.0; }
    double feet() { return _GPS_FEET_PER_METER * value() / 100.0; }
};

struct TinyGPSHDOP : TinyGPSDecimal
{
    double hdop() { return value() / 100.0; }
};

class X_GNSS
{
public:
    X_GNSS() : rtkParse(nullptr){

    };
    ~X_GNSS();

    void decode(char c);

    TinyGPSLocation location;
    TinyGPSDate date;
    TinyGPSTime time;
    TinyGPSSpeed speed;
    TinyGPSCourse course;
    TinyGPSAltitude altitude;
    //TinyGPSInteger satellites;
    TinyGPSHDOP hdop;

private:
    SEMP_PARSE_STATE *rtkParse;
};

#endif