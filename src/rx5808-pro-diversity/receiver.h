#ifndef RECEIVER_H
#define RECEIVER_H


#include <stdint.h>
#include "settings.h"


#define RECEIVER_LAST_DELAY 50
#define RECEIVER_LAST_DATA_SIZE 8


namespace Receiver {
    enum class ReceiverId : uint8_t {
        A
        #ifdef USE_DIVERSITY
            ,
            B
            #ifdef USE_6X_DIVERSITY
                ,
                C,
                D,
                E,
                F
            #endif
        #endif
    };

    #ifdef USE_DIVERSITY
        enum class DiversityMode : uint8_t {
            AUTO,
            FORCE_A,
            FORCE_B

            #ifdef USE_6X_DIVERSITY
                ,
                FORCE_C,
                FORCE_D,
                FORCE_E,
                FORCE_F
            #endif
        };
    #endif


    extern ReceiverId activeReceiver;
    extern uint8_t activeChannel;

    extern uint8_t rssiA;
    extern uint16_t rssiARaw;
    extern uint8_t rssiALast[RECEIVER_LAST_DATA_SIZE];

    #ifdef USE_DIVERSITY
        extern uint8_t rssiB;
        extern uint16_t rssiBRaw;
        extern uint8_t rssiBLast[RECEIVER_LAST_DATA_SIZE];

        #ifdef USE_6X_DIVERSITY
            extern uint8_t rssiC;
            extern uint16_t rssiCRaw;
            extern uint8_t rssiCLast[RECEIVER_LAST_DATA_SIZE];

            extern uint8_t rssiD;
            extern uint16_t rssiDRaw;
            extern uint8_t rssiDLast[RECEIVER_LAST_DATA_SIZE];

            extern uint8_t rssiE;
            extern uint16_t rssiERaw;
            extern uint8_t rssiELast[RECEIVER_LAST_DATA_SIZE];

            extern uint8_t rssiF;
            extern uint16_t rssiFRaw;
            extern uint8_t rssiFLast[RECEIVER_LAST_DATA_SIZE];
        #endif
    #endif

    void setChannel(uint8_t channel);
    uint16_t updateRssi();
    void setActiveReceiver(ReceiverId receiver = ReceiverId::A);
    #ifdef USE_DIVERSITY
        void setDiversityMode(uint8_t mode);
        void switchDiversity();
    #endif

    bool isRssiStable();


    void setup();
    void update();
}


#endif
