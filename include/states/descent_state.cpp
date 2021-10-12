#pragma once

#include "Arduino.h"
#include "core/core.cpp"
#include "flight_state.cpp"
#include "communication.h"
#include "buzzer.h"
#include "magnetometer_wrapper.h"
#include "gps_wrapper.h"
#include "barometer_wrapper.h"

class DescentState : public State
{
private:
    int showMessage = 1;

public:
    void start() override
    {
        String msg = "proof of concept --- DESCENT STATE";
        Serial.println(msg);

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

            Serial.println("Looping in descent state!");
        }
    }

    void HandleNextPhase() override
    {
        if (showMessage)
        {
            Serial.println("proof of concept --- END of proof of concept");
        }
        showMessage = 0;
    }
};
