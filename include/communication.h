#pragma once
#include <Arduino.h>
#include <cstdio>
#include <ArduinoJson.h>
#include "sensor_data.h"
#include "lora_wrapper.h"
#include "thread_wrapper.h"

using namespace std;

namespace comms
{
    String serializeData();
    void loop(void *args);

    void setup(double frequency = 868E6)
    {
        lora::setup(frequency);
        Serial.println("LoRa initialized!");
        s_thread::setup(loop);
        
    }

    // This is ran in a seperate thread
    void loop(void *args = NULL)
    {
        // Send lora every 0.4 secs
        while (true)
        {
            String serialized = comms::serializeData();
            lora::sendMessage(serialized, s_data.lora_message_id);
            Serial.print("Lora (msg id: ");
            Serial.print(s_data.lora_message_id);
            Serial.print(") sent: ");
            Serial.println(serialized);
            s_data.lora_message_id++;
            delay(400);
        }
    }

    String serializeData()
    {
        char outgoing[80];
        static int counter = 0;
        sens_data::GpsData gps = s_data.getGpsData();
        sens_data::MagenetometerData mag = s_data.getMagnetometerData();
        sens_data::BarometerData bar = s_data.getBarometerData();
        sprintf(outgoing, "%7.4f,%7.4f,%5.0f,%5.2f,%5.2f,%5.2f,%3.1f,%5.1f,%6.1f,%4d", gps.lat, gps.lng, gps.alt, mag.x, mag.y, mag.z, bar.temperature, bar.pressure, bar.altitude, counter);
        counter++;
        return outgoing;
    }

}