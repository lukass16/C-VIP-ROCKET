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
#include "EEPROM.h" //*NEW

class PreperationState: public State {
    public: 
        void start () override {
            Serial.println("proof of concept --- PREP STATE");
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

            //TODO if magnetometer already calibrated still waiting to retrieve values - sets new

            if(!magnetometer::savedCorToEEPROM())
            {
                buzzer::signalCalibrationStart();
                magnetometer::calibrate(1);
                buzzer::signalCalibrationEnd();
            }
            else
            {
                Serial.println("Magnetometer has already been calibrated - skipping calibration process");
                buzzer::signalCalibrationStart();
                buzzer::signalCalibrationEnd();
            }

            //! Checks second switch with safety against fast pull
            while(!arming::armingSuccess())
            {
                if(arming::checkSecondSwitch() && arming::timeKeeper && arming::fail == 0)
                {
                    magnetometer::saveCorToEEPROM();
                    arming::AlreadyCalibrated = 1;  
                }
                else if (arming::checkSecondSwitch() && !arming::timeKeeper)
                {                                                                   
                    arming::fail = 1;                                                          
                    Serial.println("CALIBRATION FAILED, AFFIRMED TOO FAST"); 
                }   
            }

            //TODO add 10 second loop (try elegant) after which save eeprom cor and signal
            magnetometer::getCorEEPROM();
            //TODO end

      
            //* permanent loop while not successfull arming or not pulled third switch
            while(!arming::armingSuccess() || !arming::checkThirdSwitch())
            {
                if(arming::armingSuccess() && arming::checkThirdSwitch())
                {
                    this->_context->RequestNextPhase();
                    this->_context->Start();
                }
            }
           
        }

        void HandleNextPhase() override {
            Serial.println("proof of concept --- NEXT STATE for PREP");
            this->_context->TransitionTo(new FlightState);
        }
};