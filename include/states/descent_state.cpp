#pragma once

#include "Arduino.h"
#include "core/core.cpp"
#include "flight_state.cpp"
#include "communication.h"
#include "buzzer.h"
#include "magnetometer_wrapper.h"
#include "gps_wrapper.h"
#include "barometer_wrapper.h"
#include "flash.h"

class DescentState : public State {
    public:
        void start() override
        {
            Serial.println("DESCENT STATE");

            unsigned long start_time_descent = millis();
            int descent_write_time = 60000; //ms
            bool file_closed = 0;
            File file = flash::openFile();

            while (true)
            {
                buzzer::signalDescent();

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

                if(millis() - start_time_descent < descent_write_time)
                {
                    flash::writeData(file, gd, md, bd); //writing data to flash memory
                }
                else if(!file_closed)
                {
                    flash::closeFile(file); //closing flash file
                    Serial.println("Flash data file closed");
                    file_closed = 1;
                }  
            }
        }

        void HandleNextPhase() override
        {
            Serial.println("END of VIP ROCKET CODE");
        }
};
