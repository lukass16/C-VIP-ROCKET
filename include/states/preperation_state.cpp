#pragma once

#include "Arduino.h"
#include "communication.h"
#include "barometer_wrapper.h"
#include "magnetometer_wrapper.h"
#include "buzzer.h"
#include "core/core.cpp"
#include "flight_state.cpp"
#include "test_state.cpp"
#include "arming.h"
#include "EEPROM.h"

class PreperationState: public State {
    public: 
        void start () override {
            Serial.println("PREP STATE");
            buzzer::signalStart();

            arming::setup();
            Wire.begin(12, 13);

            buzzer::setup();
            buzzer::test();
            gps::setup(9600);            
            barometer::setup();
            magnetometer::setup();
            comms::setup(868E6);

            //magnetometer::clearEEPROM();
            magnetometer::getCorEEPROM();
            magnetometer::displayCor();

            if(magnetometer::hasBeenLaunch())
            {
                this->_context->RequestNextPhase(); //! Transition to flight state
                this->_context->Start();
            }

            if(!magnetometer::savedCorToEEPROM())
            {
                buzzer::signalCalibrationStart();
                magnetometer::calibrate();
                buzzer::signalCalibrationEnd();
            }
            else
            {
                Serial.println("Calibration skipped - EEPROM shows as calibrated");
                buzzer::signalCalibrationStart();
                buzzer::signalCalibrationEnd();
            }

            arming::secondSwitchStart = millis();
            while(!arming::armingSuccess() && !magnetometer::savedCorToEEPROM())
            {
                if(!arming::checkFirstSwitch() && !arming::firstSwitchHasBeen)
                {
                    buzzer::buzz(1080);
                    delay(1000);
                    buzzer::buzzEnd();
                    arming::firstSwitchHasBeen = 1;
                }
                if(arming::checkSecondSwitch() && !arming::timeKeeper)
                {                                                                   
                    arming::fail = 1;                                                          
                    Serial.println("Calibration failed, affirmed too fast!"); 
                } 
                else if(arming::checkSecondSwitch() && arming::checkThirdSwitch()) //ja ir izvilkts slēdzis 
                {
                    buzzer::signalSecondSwitch();
                    if(millis() - arming::secondSwitchStart > 10000) //un ja pagājis vairāk kā noteiktais intervāls
                    {
                        arming::AlreadyCalibrated = 1;
                        magnetometer::saveCorToEEPROM();
                        magnetometer::setAsCalibrated();
                        Serial.println("EEPROM calibration values saved");
                        buzzer::signalSavedValues();
                    } 
                }
                else
                {
                    arming::secondSwitchStart = millis(); //resetto izvilkšanas sākuma laiku uz pašreizējo laiku
                }
            }
            magnetometer::getCorEEPROM();
            magnetometer::displayCor();

            //permanent loop while not pulled third switch
            while(!arming::checkSecondSwitch() || arming::checkThirdSwitch()) {}
            this->_context->RequestNextPhase();
            this->_context->Start();
           
        }

        void HandleNextPhase() override {
            Serial.println("proof of concept --- NEXT STATE for PREP");
            this->_context->TransitionTo(new FlightState);
        }
};