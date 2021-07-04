#pragma once

#include "Arduino.h"
#include "core/core.cpp"
#include "flight_state.cpp"
#include "communication.h"
#include "buzzer.h"
#include "magnetometer_wrapper.h"
#include "gps_wrapper.h"
#include "barometer_wrapper.h"

class DescentState: public State {
    private:
        int showMessage = 1;

    public: 
        void start () override {
            String msg = "proof of concept --- DESCENT STATE";
            Serial.println(msg);
            lora::sendMessage(msg, 777);

            buzzer::buzz(0);

            while (true)
            {

                gps::readGps();
                barometer::readSensor();
                
                if(gps::hasData) {
                   sens_data::GpsData gd = gps::getGpsState();
                   s_data.setGpsData(gd);
                }

                magnetometer::readMagnetometer();
                
                Serial.println("Looping in descent state!");

                //TODO add flash

            }
        }

        void HandleNextPhase() override {
            if(showMessage) {
                Serial.println("proof of concept --- END of proof of concept");
            }
            showMessage = 0;
        }
};
