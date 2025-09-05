#ifndef __X_GNSS_H
#define __X_GNSS_H
#include "Arduino.h"
#include "SparkFun_Extensible_Message_Parser.h"

#define CONFIG_GPS_DEBUG 1
#if defined(CONFIG_GPS_DEBUG)
#define GPS_LOG_INFO(format, ...)  log_i("[GPS] [Info] " format, ##__VA_ARGS__)
#define GPS_LOG_WARN(format, ...)  log_w("[GPS] [Warn] " format, ##__VA_ARGS__)
#define GPS_LOG_ERROR(format, ...) log_e("[GPS] [Error] " format, ##__VA_ARGS__)
#else
#define GPS_LOG_INFO(...)
#define GPS_LOG_WARN(...)
#define GPS_LOG_ERROR(...)
#endif

#define _GPS_VERSION "1.0.0" // software version of this library
#define _GPS_MPH_PER_KNOT 1.15077945
#define _GPS_MPS_PER_KNOT 0.51444444
#define _GPS_KMPH_PER_KNOT 1.852
#define _GPS_MILES_PER_METER 0.00062137112
#define _GPS_KM_PER_METER 0.001
#define _GPS_FEET_PER_METER 3.2808399
#define _GPS_MAX_FIELD_SIZE 15
#define _GPS_EARTH_MEAN_RADIUS 6371009 // old: 6372795

// COMBINE宏用于将句子类型和字段编号组合成唯一标识符
#define COMBINE(sentence_type, term_number) (((unsigned)(sentence_type) << 5) | term_number)

class X_GNSS;

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
    void setLatitude(const char *term);
    void setLongitude(const char *term);
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
    uint16_t year()
    {
        updated = false;
        uint16_t year = date % 100;
        return year + 2000;
    }
    uint8_t month()
    {
        updated = false;
        return (date / 100) % 100;
    }
    uint8_t day()
    {
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
    void setDate(const char *term)
    {
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
        if (negative)
            ++term;
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
    void commit()
    {
        val = newval;
        lastCommitTime = millis();
        valid = updated = true;
    }
    void set(const char *term)
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
        newval = negative ? -ret : ret;
    }
};

struct TinyGPSInteger
{
    friend class X_GNSS;

public:
    bool isValid() const { return valid; }
    bool isUpdated() const { return updated; }
    uint32_t age() const { return valid ? millis() - lastCommitTime : (uint32_t)ULONG_MAX; }
    uint32_t value()
    {
        updated = false;
        return val;
    }

    TinyGPSInteger() : valid(false), updated(false), val(0)
    {
    }

private:
    bool valid, updated;
    uint32_t lastCommitTime;
    uint32_t val, newval;
    void commit()
    {
        val = newval;
        lastCommitTime = millis();
        valid = updated = true;
    }
    void set(const char *term)
    {
        newval = atol(term);
    }
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
    enum
    {
        GPS_SENTENCE_GGA,
        GPS_SENTENCE_RMC,
        GPS_SENTENCE_OTHER
    };

    X_GNSS();
    ~X_GNSS()
    {
        if (rtkParse)
            sempStopParser(&rtkParse);

        rtkParse = nullptr;
    }

    void begin(SEMP_PARSE_STATE *rtkParse)
    {
        this->rtkParse = rtkParse;
    }

    void decode(char c);
    void nemaHandler(uint8_t *response, uint16_t length);
    void parseCompleteSentence(const char *sentence);
    void parseTerm(const char *term);

    void commitAll();
    TinyGPSLocation location;
    TinyGPSDate date;
    TinyGPSTime time;
    TinyGPSSpeed speed;
    TinyGPSCourse course;
    TinyGPSAltitude altitude;
    TinyGPSInteger satellites;
    TinyGPSHDOP hdop;

    static const char *libraryVersion() { return _GPS_VERSION; }
    void getFirmwareVersion(char *p_buf)
    {
        strcpy(p_buf, this->firmwareVersion);
    }

    static double distanceBetween(double lat1, double long1, double lat2, double long2);
    static double courseTo(double lat1, double long1, double lat2, double long2);
    static const char *cardinal(double course);

    static int32_t parseDecimal(const char *term);
    static void parseDegrees(const char *term, RawDegrees &deg);

public:
    uint8_t curSentenceType;
    uint8_t curTermNumber;
    bool sentenceHasFix;
    char firmwareVersion[16];

private:
    SEMP_PARSE_STATE *rtkParse;
};

#endif