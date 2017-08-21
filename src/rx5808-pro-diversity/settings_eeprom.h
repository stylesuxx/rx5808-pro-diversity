#ifndef EEPROM_SETTINGS_H
#define EEPROM_SETTINGS_H


#include <stdint.h>
#include <avr/pgmspace.h>

#include "settings.h"
#include "settings_internal.h"
#include "receiver.h"


struct EepromSettings {
    uint32_t magic;
    uint8_t startChannel;

    uint8_t beepEnabled;

    uint8_t searchManual;
    uint8_t searchOrderByChannel;

    uint16_t rssiAMin;
    uint16_t rssiAMax;

    #ifdef USE_DIVERSITY
        Receiver::DiversityMode diversityMode;
        uint16_t rssiBMin;
        uint16_t rssiBMax;

        #ifdef USE_6X_DIVERSITY
            uint16_t rssiCMin;
            uint16_t rssiCMax;

            uint16_t rssiDMin;
            uint16_t rssiDMax;

            uint16_t rssiEMin;
            uint16_t rssiEMax;

            uint16_t rssiFMin;
            uint16_t rssiFMax;
        #endif
    #endif

    #ifdef USE_VOLTAGE_MONITORING
        uint8_t vbatScale;
        uint8_t vbatWarning;
        uint8_t vbatCritical;
    #endif


    void update();

    void load();
    void save();
    void markDirty();

    void initDefaults();
};


PROGMEM const struct {
    uint32_t magic = EEPROM_MAGIC;
    uint8_t startChannel = 0;

    uint8_t beepEnabled = true;

    uint8_t searchManual = false;
    uint8_t searchOrderByChannel = false;

    uint16_t rssiAMin = RSSI_MIN_VAL;
    uint16_t rssiAMax = RSSI_MAX_VAL;

    #ifdef USE_DIVERSITY
        Receiver::DiversityMode diversityMode = Receiver::DiversityMode::AUTO;
        uint16_t rssiBMin = RSSI_MIN_VAL;
        uint16_t rssiBMax = RSSI_MAX_VAL;

        #ifdef USE_6X_DIVERSITY
            uint16_t rssiCMin = RSSI_MIN_VAL;
            uint16_t rssiCMax = RSSI_MAX_VAL;

            uint16_t rssiDMin = RSSI_MIN_VAL;
            uint16_t rssiDMax = RSSI_MAX_VAL;

            uint16_t rssiEMin = RSSI_MIN_VAL;
            uint16_t rssiEMax = RSSI_MAX_VAL;

            uint16_t rssiFMin = RSSI_MIN_VAL;
            uint16_t rssiFMax = RSSI_MAX_VAL;
        #endif
    #endif

    #ifdef USE_VOLTAGE_MONITORING
        uint8_t vbatScale = VBAT_SCALE;
        uint8_t vbatWarning = WARNING_VOLTAGE;
        uint8_t vbatCritical = CRITICAL_VOLTAGE;
    #endif
} EepromDefaults;


extern EepromSettings EepromSettings;


#endif
