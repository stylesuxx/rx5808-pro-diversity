#include <Arduino.h>
#include <avr/pgmspace.h>

#include "settings.h"
#include "settings_eeprom.h"
#include "receiver.h"
#include "receiver_spi.h"
#include "channels.h"

#include "timer.h"

static void updateRssiLimits();
static void writeSerialData();


namespace Receiver {
    ReceiverId activeReceiver = ReceiverId::A;
    uint8_t activeChannel = 0;

    uint8_t rssiA = 0;
    uint16_t rssiARaw = 0;
    uint8_t rssiALast[RECEIVER_LAST_DATA_SIZE] = { 0 };

    #ifdef USE_DIVERSITY
        uint8_t rssiB = 0;
        uint16_t rssiBRaw = 0;
        uint8_t rssiBLast[RECEIVER_LAST_DATA_SIZE] = { 0 };

        #ifdef USE_6X_DIVERSITY
            uint8_t rssiC = 0;
            uint16_t rssiCRaw = 0;
            uint8_t rssiCLast[RECEIVER_LAST_DATA_SIZE] = { 0 };

            uint8_t rssiD = 0;
            uint16_t rssiDRaw = 0;
            uint8_t rssiDLast[RECEIVER_LAST_DATA_SIZE] = { 0 };

            uint8_t rssiE = 0;
            uint16_t rssiERaw = 0;
            uint8_t rssiELast[RECEIVER_LAST_DATA_SIZE] = { 0 };

            uint8_t rssiF = 0;
            uint16_t rssiFRaw = 0;
            uint8_t rssiFLast[RECEIVER_LAST_DATA_SIZE] = { 0 };
        #endif

        ReceiverId diversityTargetReceiver = activeReceiver;
        Timer diversityHysteresisTimer = Timer(DIVERSITY_HYSTERESIS_PERIOD);
    #endif

    static Timer rssiStableTimer = Timer(MIN_TUNE_TIME);
    static Timer rssiLogTimer = Timer(RECEIVER_LAST_DELAY);
    #ifdef USE_SERIAL_OUT
        static Timer serialLogTimer = Timer(25);
    #endif


    void setChannel(uint8_t channel)
    {
        ReceiverSpi::setSynthRegisterB(Channels::getSynthRegisterB(channel));

        rssiStableTimer.reset();
        activeChannel = channel;
    }

    void setActiveReceiver(ReceiverId receiver) {
        #ifdef USE_DIVERSITY
            #ifdef USE_DIVERSITY_FAST_SWITCHING
                uint8_t mask, maskDisabled;
                if (receiver == ReceiverId::A) {
                    mask = MASK_RECEIVER_A;
                    maskDisabled = MASK_RECEIVER_B;
                } else {
                    mask = MASK_RECEIVER_B;
                    maskDisabled = MASK_RECEIVER_A;
                }

                uint8_t port = digitalPinToPort(PIN_SWITCH_0);
                volatile uint8_t *out = portOutputRegister(port);

                *out = (*out | mask) & ~maskDisabled;
            #else
                digitalWrite(PIN_SWITCH_0, receiver == ReceiverId::A);
                digitalWrite(PIN_SWITCH_1, receiver == ReceiverId::B);
            #endif
        #else
            digitalWrite(PIN_SWITCH_0, HIGH);
        #endif

        activeReceiver = receiver;
    }

    bool isRssiStable() {
        return rssiStableTimer.hasTicked();
    }

    uint16_t updateRssi() {
        analogRead(PIN_RSSI_A); // Fake read to let ADC settle.
        rssiARaw = analogRead(PIN_RSSI_A);
        #ifdef USE_DIVERSITY
            analogRead(PIN_RSSI_B);
            rssiBRaw = analogRead(PIN_RSSI_B);

            #ifdef USE_6X_DIVERSITY
                analogRead(PIN_RSSI_C);
                rssiCRaw = analogRead(PIN_RSSI_C);

                analogRead(PIN_RSSI_D);
                rssiDRaw = analogRead(PIN_RSSI_D);

                analogRead(PIN_RSSI_E);
                rssiERaw = analogRead(PIN_RSSI_E);

                analogRead(PIN_RSSI_F);
                rssiFRaw = analogRead(PIN_RSSI_F);
            #endif
        #endif

        rssiA = constrain(
            map(
                rssiARaw,
                EepromSettings.rssiAMin,
                EepromSettings.rssiAMax,
                0,
                100
            ),
            0,
            100
        );
        #ifdef USE_DIVERSITY
            rssiB = constrain(
                map(
                    rssiBRaw,
                    EepromSettings.rssiBMin,
                    EepromSettings.rssiBMax,
                    0,
                    100
                ),
                0,
                100
            );

            #ifdef USE_6X_DIVERSITY
                rssiC = constrain(
                    map(
                        rssiCRaw,
                        EepromSettings.rssiCMin,
                        EepromSettings.rssiCMax,
                        0,
                        100
                    ),
                    0,
                    100
                );

                rssiD = constrain(
                    map(
                        rssiDRaw,
                        EepromSettings.rssiDMin,
                        EepromSettings.rssiDMax,
                        0,
                        100
                    ),
                    0,
                    100
                );
                rssiE = constrain(
                    map(
                        rssiERaw,
                        EepromSettings.rssiEMin,
                        EepromSettings.rssiEMax,
                        0,
                        100
                    ),
                    0,
                    100
                );

                rssiF = constrain(
                    map(
                        rssiFRaw,
                        EepromSettings.rssiFMin,
                        EepromSettings.rssiFMax,
                        0,
                        100
                    ),
                    0,
                    100
                );
            #endif
        #endif

        if (rssiLogTimer.hasTicked()) {
            for (uint8_t i = 0; i < RECEIVER_LAST_DATA_SIZE - 1; i++) {
                rssiALast[i] = rssiALast[i + 1];
                #ifdef USE_DIVERSITY
                    rssiBLast[i] = rssiBLast[i + 1];

                    #ifdef USE_6X_DIVERSITY
                        rssiCLast[i] = rssiCLast[i + 1];
                        rssiDLast[i] = rssiDLast[i + 1];
                        rssiELast[i] = rssiELast[i + 1];
                        rssiFLast[i] = rssiFLast[i + 1];
                    #endif
                #endif
            }

            rssiALast[RECEIVER_LAST_DATA_SIZE - 1] = rssiA;
            #ifdef USE_DIVERSITY
                rssiBLast[RECEIVER_LAST_DATA_SIZE - 1] = rssiB;

                #ifdef USE_6X_DIVERSITY
                    rssiCLast[RECEIVER_LAST_DATA_SIZE - 1] = rssiC;
                    rssiDLast[RECEIVER_LAST_DATA_SIZE - 1] = rssiD;
                    rssiELast[RECEIVER_LAST_DATA_SIZE - 1] = rssiE;
                    rssiFLast[RECEIVER_LAST_DATA_SIZE - 1] = rssiF;
                #endif
            #endif

            rssiLogTimer.reset();
        }
    }

#ifdef USE_DIVERSITY
    void setDiversityMode(DiversityMode mode) {
        EepromSettings.diversityMode = mode;
        switchDiversity();
    }

    // TODO: ADAPT for six receivers
    void switchDiversity() {
        ReceiverId nextReceiver = activeReceiver;

        if (EepromSettings.diversityMode == DiversityMode::AUTO) {
            int8_t rssiDiff = (int8_t) rssiA - (int8_t) rssiB;
            uint8_t rssiDiffAbs = abs(rssiDiff);
            ReceiverId currentBestReceiver = activeReceiver;

            if (rssiDiff > 0) {
                currentBestReceiver = ReceiverId::A;
            } else if (rssiDiff < 0) {
                currentBestReceiver = ReceiverId::B;
            } else {
                currentBestReceiver = activeReceiver;
            }

            if (rssiDiffAbs >= DIVERSITY_HYSTERESIS) {
                if (currentBestReceiver == diversityTargetReceiver) {
                    if (diversityHysteresisTimer.hasTicked()) {
                        nextReceiver = diversityTargetReceiver;
                    }
                } else {
                    diversityTargetReceiver = currentBestReceiver;
                    diversityHysteresisTimer.reset();
                }
            } else {
                diversityHysteresisTimer.reset();
            }
        } else {
            switch (EepromSettings.diversityMode) {
                case DiversityMode::FORCE_A:
                    nextReceiver = ReceiverId::A;
                    break;

                case DiversityMode::FORCE_B:
                    nextReceiver = ReceiverId::B;
                    break;

                #ifdef USE_6X_DIVERSITY
                    case DiversityMode::FORCE_C:
                        nextReceiver = ReceiverId::C;
                        break;

                    case DiversityMode::FORCE_D:
                        nextReceiver = ReceiverId::D;
                        break;

                    case DiversityMode::FORCE_E:
                        nextReceiver = ReceiverId::E;
                        break;

                    case DiversityMode::FORCE_F:
                        nextReceiver = ReceiverId::F;
                        break;
                #endif
            }
        }

        setActiveReceiver(nextReceiver);
    }
#endif

    void setup() {
        #ifdef DISABLE_AUDIO
            ReceiverSpi::setPowerDownRegister(0b00010000110111110011);
        #endif
    }

    void update() {
        if (rssiStableTimer.hasTicked()) {
            updateRssi();

            #ifdef USE_SERIAL_OUT
                writeSerialData();
            #endif

            #ifdef USE_DIVERSITY
                switchDiversity();
            #endif
        }
    }
}


#ifdef USE_SERIAL_OUT

#include "pstr_helper.h"

static void writeSerialData() {
    if (Receiver::serialLogTimer.hasTicked()) {
        Serial.print(Receiver::activeChannel, DEC);
        Serial.print(PSTR2("\t"));
        Serial.print(Receiver::rssiA, DEC);
        Serial.print(PSTR2("\t"));
        Serial.print(Receiver::rssiARaw, DEC);

        #ifdef USE_DIVERSITY
            Serial.print(PSTR2("\t"));
            Serial.print(Receiver::rssiB, DEC);
            Serial.print(PSTR2("\t"));
            Serial.print(Receiver::rssiBRaw, DEC);

            #ifdef USE_6X_DIVERSITY
                Serial.print(PSTR2("\t"));
                Serial.print(Receiver::rssiC, DEC);
                Serial.print(PSTR2("\t"));
                Serial.print(Receiver::rssiCRaw, DEC);

                Serial.print(PSTR2("\t"));
                Serial.print(Receiver::rssiD, DEC);
                Serial.print(PSTR2("\t"));
                Serial.print(Receiver::rssiDRaw, DEC);

                Serial.print(PSTR2("\t"));
                Serial.print(Receiver::rssiE, DEC);
                Serial.print(PSTR2("\t"));
                Serial.print(Receiver::rssiERaw, DEC);

                Serial.print(PSTR2("\t"));
                Serial.print(Receiver::rssiF, DEC);
                Serial.print(PSTR2("\t"));
                Serial.println(Receiver::rssiFRaw, DEC);
            #endif
        #endif

        Receiver::serialLogTimer.reset();
    }
}
#endif
