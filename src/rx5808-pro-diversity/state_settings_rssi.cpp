#include <stdint.h>

#include "state_settings_rssi.h"

#include "receiver.h"
#include "channels.h"

#include "settings.h"
#include "settings_internal.h"
#include "settings_eeprom.h"
#include "buttons.h"

#include "ui.h"
#include "pstr_helper.h"


void StateMachine::SettingsRssiStateHandler::onEnter() {
    internalState = InternalState::WAIT_FOR_LOW;
}

void StateMachine::SettingsRssiStateHandler::onUpdate() {
    if (!Receiver::isRssiStable())
        return;

    switch (internalState) {
        case InternalState::SCANNING_LOW:
            if (Receiver::rssiARaw < EepromSettings.rssiAMin)
                EepromSettings.rssiAMin = Receiver::rssiARaw;

            #ifdef USE_DIVERSITY
                if (Receiver::rssiBRaw < EepromSettings.rssiBMin)
                    EepromSettings.rssiBMin = Receiver::rssiBRaw;

                #ifdef USE_6X_DIVERSITY
                    if (Receiver::rssiCRaw < EepromSettings.rssiCMin)
                        EepromSettings.rssiCMin = Receiver::rssiCRaw;

                    if (Receiver::rssiDRaw < EepromSettings.rssiDMin)
                        EepromSettings.rssiDMin = Receiver::rssiDRaw;

                    if (Receiver::rssiERaw < EepromSettings.rssiEMin)
                        EepromSettings.rssiEMin = Receiver::rssiERaw;

                    if (Receiver::rssiFRaw < EepromSettings.rssiFMin)
                        EepromSettings.rssiFMin = Receiver::rssiFRaw;
                #endif
            #endif
        break;

        case InternalState::SCANNING_HIGH:
            if (Receiver::rssiARaw > EepromSettings.rssiAMax)
                EepromSettings.rssiAMax = Receiver::rssiARaw;

            #ifdef USE_DIVERSITY
                if (Receiver::rssiBRaw > EepromSettings.rssiBMax)
                    EepromSettings.rssiBMax = Receiver::rssiBRaw;

                #ifdef USE_6X_DIVERSITY
                    if (Receiver::rssiCRaw > EepromSettings.rssiCMax)
                        EepromSettings.rssiCMax = Receiver::rssiCRaw;

                    if (Receiver::rssiDRaw > EepromSettings.rssiDMax)
                        EepromSettings.rssiDMax = Receiver::rssiDRaw;

                    if (Receiver::rssiERaw > EepromSettings.rssiEMax)
                        EepromSettings.rssiEMax = Receiver::rssiERaw;

                    if (Receiver::rssiFRaw > EepromSettings.rssiFMax)
                        EepromSettings.rssiFMax = Receiver::rssiFRaw;
                #endif
            #endif
        break;
    }

    Receiver::setChannel((Receiver::activeChannel + 1) % CHANNELS_SIZE);
    if (Receiver::activeChannel == 0) {
        currentSweep++;

        if (currentSweep == RSSI_SETUP_RUN) {
            switch (internalState) {
                case InternalState::SCANNING_LOW:
                    internalState = InternalState::WAIT_FOR_HIGH;
                break;

                case InternalState::SCANNING_HIGH:
                    internalState = InternalState::DONE;
                break;
            }

            Ui::needUpdate();
        }
    }
}

void StateMachine::SettingsRssiStateHandler::onButtonChange(
    Button button,
    Buttons::PressType pressType
) {
    if (button != Button::MODE || pressType != Buttons::PressType::SHORT)
        return;

    switch (internalState) {
        case InternalState::WAIT_FOR_LOW:
            internalState = InternalState::SCANNING_LOW;
            currentSweep = 0;
            Receiver::setChannel(0);

            EepromSettings.rssiAMin = UINT16_MAX;
            #ifdef USE_DIVERSITY
                EepromSettings.rssiBMin = UINT16_MAX;
                    #ifdef USE_6X_DIVERSITY
                        EepromSettings.rssiCMin = UINT16_MAX;
                        EepromSettings.rssiDMin = UINT16_MAX;
                        EepromSettings.rssiEMin = UINT16_MAX;
                        EepromSettings.rssiFMin = UINT16_MAX;
                    #endif
            #endif
        break;

        case InternalState::WAIT_FOR_HIGH:
            internalState = InternalState::SCANNING_HIGH;
            currentSweep = 0;
            Receiver::setChannel(0);

            EepromSettings.rssiAMax = 0;
            #ifdef USE_DIVERSITY
                EepromSettings.rssiBMax = 0;
                #ifdef USE_6X_DIVERSITY
                    EepromSettings.rssiCMax = 0;
                    EepromSettings.rssiDMax = 0;
                    EepromSettings.rssiEMax = 0;
                    EepromSettings.rssiFMax = 0;
                #endif
            #endif
        break;

        case InternalState::DONE:
            EepromSettings.save();
            StateMachine::switchState(StateMachine::State::MENU);
        break;
    }

    Ui::needUpdate();
}


void StateMachine::SettingsRssiStateHandler::onInitialDraw() {
    Ui::needUpdate(); // Lazy. :(
}

void StateMachine::SettingsRssiStateHandler::onUpdateDraw() {
    Ui::clear();

    switch (internalState) {
        case InternalState::WAIT_FOR_LOW:
            Ui::display.setTextSize(1);
            Ui::display.setCursor(0, 0);
            Ui::display.print(PSTR2("1/4\nTurn off all VTXs."));
            Ui::display.setCursor(0, (CHAR_HEIGHT + 1) * 2);
            Ui::display.print(PSTR2("Remove RX antennas."));

            Ui::display.setCursor(0, SCREEN_HEIGHT - CHAR_HEIGHT - 1);
            Ui::display.print(PSTR2("Press MODE when ready."));
        break;

        case InternalState::SCANNING_LOW:
            Ui::display.setTextSize(1);
            Ui::display.setCursor(0, 0);
            Ui::display.print(PSTR2("2/4\nScanning for lowest\nRSSI..."));
        break;

        case InternalState::WAIT_FOR_HIGH:
            Ui::display.setTextSize(1);
            Ui::display.setCursor(0, 0);
            Ui::display.print(PSTR2("3/4\nTurn on your VTX."));

            Ui::display.setCursor(0, SCREEN_HEIGHT - CHAR_HEIGHT - 1);
            Ui::display.print(PSTR2("Press MODE when ready."));
        break;

        case InternalState::SCANNING_HIGH:
            Ui::display.setTextSize(1);
            Ui::display.setCursor(0, 0);
            Ui::display.print(PSTR2("4/4\nScanning for highest\nRSSI..."));
        break;

        case InternalState::DONE:
            Ui::display.setTextSize(1);
            Ui::display.setCursor(0, 0);
            Ui::display.print(PSTR2("All done!"));

            Ui::display.setCursor(0, CHAR_HEIGHT * 2);
            Ui::display.print(PSTR2("Min: "));

            Ui::display.setCursor((CHAR_WIDTH + 1) * 5, CHAR_HEIGHT * 2);
            Ui::display.print(EepromSettings.rssiAMin);
            #ifdef USE_DIVERSITY
                Ui::display.setCursor((CHAR_WIDTH + 1) * 10, CHAR_HEIGHT * 2);
                Ui::display.print(EepromSettings.rssiBMin);

                #ifdef USE_6X_DIVERSITY
                    Ui::display.setCursor((CHAR_WIDTH + 1) * 15,
                        CHAR_HEIGHT * 2);
                    Ui::display.print(EepromSettings.rssiCMin);

                    Ui::display.setCursor(0, CHAR_HEIGHT * 4 + 6);
                    Ui::display.print(PSTR2("Min: "));

                    Ui::display.setCursor((CHAR_WIDTH + 1) * 5,
                        CHAR_HEIGHT * 4 + 6);
                    Ui::display.print(EepromSettings.rssiDMin);

                    Ui::display.setCursor((CHAR_WIDTH + 1) * 10,
                        CHAR_HEIGHT * 4 + 6);
                    Ui::display.print(EepromSettings.rssiEMin);

                    Ui::display.setCursor((CHAR_WIDTH + 1) * 15,
                        CHAR_HEIGHT * 4 + 6);
                    Ui::display.print(EepromSettings.rssiFMin);
                #endif
            #endif

            Ui::display.setCursor(0, CHAR_HEIGHT * 3 + 1);
            Ui::display.print(PSTR2("Max: "));

            Ui::display.setCursor((CHAR_WIDTH + 1) * 5, CHAR_HEIGHT * 3 + 1);
            Ui::display.print(EepromSettings.rssiAMax);
            #ifdef USE_DIVERSITY
                Ui::display.setCursor((CHAR_WIDTH + 1) * 10,
                    CHAR_HEIGHT * 3 + 1);
                Ui::display.print(EepromSettings.rssiBMax);

                #ifdef USE_6X_DIVERSITY
                    Ui::display.setCursor((CHAR_WIDTH + 1) * 15,
                        CHAR_HEIGHT * 3 + 1);
                    Ui::display.print(EepromSettings.rssiCMax);

                    Ui::display.setCursor(0, CHAR_HEIGHT * 5 + 7);
                    Ui::display.print(PSTR2("Max: "));

                    Ui::display.setCursor((CHAR_WIDTH + 1) * 5,
                        CHAR_HEIGHT * 5 + 7);
                    Ui::display.print(EepromSettings.rssiDMax);

                    Ui::display.setCursor((CHAR_WIDTH + 1) * 10,
                        CHAR_HEIGHT * 5 + 7);
                    Ui::display.print(EepromSettings.rssiEMax);

                    Ui::display.setCursor((CHAR_WIDTH + 1) * 15,
                        CHAR_HEIGHT * 5 + 7);
                    Ui::display.print(EepromSettings.rssiFMax);
                #endif
            #endif

            Ui::display.setCursor(0, SCREEN_HEIGHT - CHAR_HEIGHT - 1);
            Ui::display.print(PSTR2("Press MODE to save."));
        break;
    }

    Ui::needDisplay();
}
