#pragma once

#include "Arduino.h"
#include "core/core.cpp"
#include "descent_state.cpp"
#include "lora_wrapper.h"
#include "gps_wrapper.h"
#include "magnetometer_wrapper.h"
#include "sensor_data.h"
#include "arming.h"


class FlightState : public State {
    private:
        volatile bool timerDetAp = 0;
        bool timerEnabled = 0;

        int magcount = 0;
        boolean isApogee()
        {
            if (magnetometer::isApogee())
            {
                magcount++;
                if (magcount>100)
                {
                    Serial.println("Magnetometer detected apogee");
                    return true;
                }
            }
            else if (magnetometer::timerDetectApogee())
            {
                return true;
                Serial.println("Timer detected apogee");
            }

            return false;
        }

    public:
        void start() override
        {
            Serial.println("FLIGHT STATE");

            //!There is danger that if launch was detected previously the rocket goes
            //!straight to arming and setting apogee timers, so if launch is detected and during testing 
            //!the launchDetected EEPROM value is not changed it could lead to an inadvertent triggering of the mechanism

            while (!isApogee())
            {
                buzzer::signalThirdSwitch();
                //While apogee isn't reached and the timer isn't yet enabled the rocket checks for launch to enable the timer - the checking of launch has no other functionality
                if (!timerEnabled)
                {
                    if (magnetometer::launch())  //checks if rocket has been launched
                    {
                        magnetometer::startApogeeTimer(14000000); //code to start timer - prints TIMER ENABLED if timer enabled
                        Serial.println("Launch detected!");
                        timerEnabled = 1;
                    }
                }

                // GPS
                gps::readGps();
                sens_data::GpsData gd;
                if (gps::hasData)
                {
                    gd = gps::getGpsState();
                    s_data.setGpsData(gd);
                }

                // MAGNETOMETER
                magnetometer::readMagnetometer();
                sens_data::MagenetometerData md = magnetometer::getMagnetometerState();
                s_data.setMagnetometerData(md);

                // BAROMETER
                sens_data::BarometerData bd = barometer::getBarometerState();
                s_data.setBarometerData(bd);
            }
            Serial.println("APOGEE DETECTED !!!");
            arming::nihromActivate();

            this->_context->RequestNextPhase();
            this->_context->Start();
        }

        void HandleNextPhase() override
        {
            Serial.println("proof of concept --- NEXT STATE for Descent");
            this->_context->TransitionTo(new DescentState());
        }
};
