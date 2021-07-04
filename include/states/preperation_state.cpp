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

            arming::setup(); //*NEW - beginning
            //! is this needed?
            EEPROM.begin(255); //*NEW 
            Wire.begin(12, 13);


            //* NEW - transition to USB test state (Changed location so as to check if USB connected before going to flight state if launch has been previously detected)
            //!Doesn't work anymore
            // if(arming::isConnectedUSB())
            // {
            //     this->_context->TransitionTo(new TestState);
            //     this->_context->Start();
            // }
            //!

            if(magnetometer::hasBeenLaunch())
            {
                this->_context->RequestNextPhase(); //! Transition to flight state
                this->_context->Start();
            }

            
            buzzer::setup();
            buzzer::test();
            gps::setup(9600);            
            barometer::setup();
            magnetometer::setup();
            comms::setup(868E6);
            

            magnetometer::calibrate(magnetometer::savedCorToEEPROM());   //*changed loc, skips if saved calibration values
            // magnetometer::testCalibratedAxis();

            //*Checks second switch with safety against fast pull
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
                    Serial.println("CALIBRATION FAILED"); 
                }   
            }
            magnetometer::getCorEEPROM();

      
            //* permanent loop while not successfull arming or not pulled third switch
            while(!arming::armingSuccess() || !arming::checkThirdSwitch())
            {
                if(arming::armingSuccess() && arming::checkThirdSwitch())
                {
                    this->_context->RequestNextPhase();
                    this->_context->Start();
                }
            }
           
           //* NEW end
        
           // this->_context->RequestNextPhase();
           // this->_context->Start();
        }

        void HandleNextPhase() override {
            Serial.println("proof of concept --- NEXT STATE for PREP");
            this->_context->TransitionTo(new FlightState);
        }
};