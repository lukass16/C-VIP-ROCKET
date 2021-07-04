#pragma once
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include "sensor_data.h"

//TODO izchekot kas notiek ar gps - kaa saglabat vecas


namespace gps {

    #define RXPIN 4
    #define TXPIN 3
    SoftwareSerial gpsSerial;
  //  SoftwareSerialConfig ssc = SWSERIAL_8N1; // 8bits-no_parity-1_stop_bit  https://github.com/plerup/espsoftwareserial/
    TinyGPSPlus gps;

    boolean hasData = false;

    //*NEW
    sens_data::GpsData lastData;  //Last data so that values of zero don't get sent when gps doesn't have lock on
    //*NEW

    void setup(uint gpsRate = 9600)
    {
  //      gpsSerial.begin(gpsRate, ssc, RXPIN, TXPIN);
        Serial.println("Init GPS: " + String(gpsRate));
    }

    void readGps()
    {
        hasData = false;

        while (gpsSerial.available())
        {
            gps.encode(gpsSerial.read());
            hasData = true;
        }

        // Serial.print("LAT=");
        // Serial.println(gps.location.lat(), 6);
        // Serial.print("LONG=");
        // Serial.println(gps.location.lng(), 6);
        // Serial.print("ALT=");
        // Serial.println(gps.altitude.meters());
    }

    double lastLatitude() {
        return gps.location.lat();  //TODO ar 6 skaitljiem?
    }

    double lastLongitude() {
        return gps.location.lng();
    }

    double lastAltitude() {
        return gps.altitude.meters();
    }

    //*NEW

    int getSatellites()
    {
        return gps.satellites.value();
    }

    //*NEW
   

    sens_data::GpsData getGpsState() {
        sens_data::GpsData gd;
        if(gps.location.isValid())
        {
            //*NEW
            //adding last good values
            lastData.lat = lastLatitude();
            lastData.lng = lastLongitude();
            lastData.alt = lastAltitude();
            gd.lat = lastLatitude();
            gd.lng = lastLongitude();
            gd.alt = lastAltitude();
            return gd;
            //*NEW
        }
        else
        {
            return lastData;
        }
        
    }
}