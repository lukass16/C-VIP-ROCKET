#pragma once
#include <Arduino.h>
#include "EEPROM.h"
#include "buzzer.h"
#include "sensor_data.h"

#define EEPROM_SIZE 255

//! the same handling function assesses the arming timer of the switch being pulled too fast and the second nihrom activation - this should be fixed

namespace arming
{

    //defining variables for first timer (timer safety for switch)
    volatile bool timeKeeper = 0;
    hw_timer_t *timer = NULL;
    portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

    void IRAM_ATTR onTimer()
    {
        portENTER_CRITICAL_ISR(&timerMux);
        timeKeeper = 1;
        portEXIT_CRITICAL_ISR(&timerMux);
    }

    //defining variables for second timer (Nihrom timer)
    volatile bool timeKeeperNihrom = 0;
    hw_timer_t *timerNihrom = NULL;
    portMUX_TYPE timerNihromMux = portMUX_INITIALIZER_UNLOCKED;

    void IRAM_ATTR onNihromTimer()
    {
        portENTER_CRITICAL_ISR(&timerNihromMux);
        timeKeeperNihrom = 1;
        portEXIT_CRITICAL_ISR(&timerNihromMux);
    }

    //pin definitions
    int nihrom = 33;            //p20 OUTPUT
    int nihrom2 = 25;           //p19 OUTPUT
    int ParachuteBattery1 = 35; //p17 INPUT
    int ParachuteBattery2 = 34; //p18 INPUT

    int FirstSwitch = 39;  //p14 INPUT
    int SecondSwitch = 38; //p16 INPUT
    int ThirdSwitch = 37;  //p15 INPUT

    const int LopyPower = 0; //!JĀDEFINĒ!
    int out = 26;            //p21 lampiņa/buzzer
    int EEPROMclear = 2;     //p8 INPUT

    bool fail = 0;
    bool AlreadyCalibrated = 0;
    bool firstSwitchHasBeen = 0;

    unsigned long secondSwitchStart = 0;

    //variables for Parachute battery voltage calculation
    float rawVoltage = 0;
    int rawReading = 0;
    float sumVoltage1 = 0;
    float sumVoltage2 = 0;
    float voltage1 = 0;
    float voltage2 = 0;
    float filteredVoltage = 0;

    //variables for Lopy battery voltage calculation
    int FirstSwitchReading = 0;
    int SecondSwitchReading = 0;
    int ThirdSwitchReading = 0;
    int rawReadingLopy = 0;
    float voltageLopy = 0;

    //variables for nihrom cycling
    int intervalNihrom = 500, iterationsNihrom = 10;
    unsigned long previousTimeFirst = 0, currentTimeFirst = 0, previousTimeSecond = 0, currentTimeSecond = 0;
    bool firstNihromActive = 0, secondNihromActive = 0, firstNihromFirstActive = 1, secondNihromFirstActive = 1;
    void setup()
    {
        pinMode(nihrom, OUTPUT);           //1. nihroma
        pinMode(nihrom2, OUTPUT);          // 2. nihromam
        pinMode(ParachuteBattery1, INPUT); //MOSFET shēmas baterijai
        pinMode(ParachuteBattery2, INPUT); //MOSFET shēmas baterijai

        pinMode(ThirdSwitch, INPUT);
        pinMode(SecondSwitch, INPUT);
        pinMode(FirstSwitch, INPUT);

        pinMode(out, OUTPUT); //? buzzer
        pinMode(EEPROMclear, INPUT_PULLDOWN);

        //EEPROM setup
        EEPROM.begin(EEPROM_SIZE);
        Serial.println("Arming setup complete!");
    }

    void startThirdSwitchTimer(int microseconds = 5000000)
    {
        //start third switch timer
        timer = timerBegin(0, 80, true);
        timerAttachInterrupt(timer, &onTimer, true);
        timerAlarmWrite(timer, microseconds, false); 
        timerAlarmEnable(timer);
    }

    float getBattery1Voltage()
    {
        //static int readings = 0;
        //readings++;
        rawReading = analogRead(ParachuteBattery1);
        voltage1 = (rawReading / 320.0);
        //sumVoltage1 += rawVoltage;
        //voltage1 = sumVoltage1/readings;
        //return voltage1;
        return voltage1;
    }

    float getBattery2Voltage()
    {
        // static int readings = 0;
        // readings++;
        rawReading = analogRead(ParachuteBattery2);
        voltage2 = (rawReading / 320.0); //!fix int/int
        // sumVoltage2 += rawVoltage;
        // voltage2 = sumVoltage2/readings;
        // return voltage2;
        return voltage2;
    }

    float getLopyBatteryVoltage()
    {
        FirstSwitchReading = analogRead(FirstSwitch);
        SecondSwitchReading = analogRead(SecondSwitch);
        ThirdSwitchReading = analogRead(ThirdSwitch);
        if (SecondSwitchReading != 0)
        {
            rawReadingLopy = SecondSwitchReading;
        }
        else
        {
            rawReadingLopy = ThirdSwitchReading;
        }

        voltageLopy = rawReadingLopy / 602.0;
        return voltageLopy;
    }

    bool getParachuteBatteryStatus()
    {
        if (voltage1 > 8.1 && voltage2 > 8.1 && voltageLopy > 4.1)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }

    bool checkFirstSwitch()
    {
        if (analogRead(FirstSwitch) > 700)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }

    bool checkSecondSwitch()
    {
        if (analogRead(SecondSwitch) > 1000)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }

    bool checkThirdSwitch()
    {
        if (analogRead(ThirdSwitch) > 1000)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }

    bool armingSuccess()
    {
        if (AlreadyCalibrated == 1 && fail == 0)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }

    void reportFirstSwitch()
    {
        if(!arming::checkFirstSwitch() && !arming::firstSwitchHasBeen)
        {
            buzzer::buzz(1080);
            delay(1000);
            buzzer::buzzEnd();
            arming::firstSwitchHasBeen = 1;
        }
    }

    bool thirdSwitchTooFast()
    {
        if(!arming::fail)
        {
            if(!arming::checkThirdSwitch() && !arming::timeKeeper)
            {                                                                   
                arming::fail = 1; 
                buzzer::buzz(2000);                                                         
                Serial.println("Transition to flight state failed, affirmed too fast!"); 
                return 1;
            }
            return 0;
        }
        else {return 1;}
    }

    bool clearEEPROM()
    {
        if (digitalRead(EEPROMclear) == HIGH)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }

    void nihromDisable()
    {
        digitalWrite(nihrom, LOW);
        digitalWrite(nihrom2, LOW);
    }

    void nihromActivateFirst()
    {
        static int iterations = 0;
        if (firstNihromFirstActive)
        {
            Serial.println("First Nihrom activated");
            timerNihrom = timerBegin(1, 80, true);
            timerAttachInterrupt(timerNihrom, &onNihromTimer, true);
            timerAlarmWrite(timerNihrom, 1000000, false); //1sek
            timerAlarmEnable(timerNihrom);
            firstNihromFirstActive = 0;
        }
        currentTimeFirst = millis();
        if (currentTimeFirst - previousTimeFirst >= intervalNihrom && iterations <= iterationsNihrom) //if enough time has passed and not at the end of cycles
        {
            previousTimeFirst = currentTimeFirst; //save the last time that the nihrom was toggled
            if (!firstNihromActive)               //if not active
            {
                digitalWrite(nihrom, HIGH);
                firstNihromActive = 1;
                //buzzer::buzz(3400);
            }
            else
            {
                digitalWrite(nihrom, LOW);
                firstNihromActive = 0;
                iterations++;
            }
        }
    }

    void nihromActivateSecond()
    {
        static int iterations = 0;
        if (timeKeeperNihrom && iterations <= iterationsNihrom) //if activated and not at the end of cycles
        {
            if (secondNihromFirstActive)
            {
                Serial.println("Second Nihrom activated");
                secondNihromFirstActive = 0;
            }
            currentTimeSecond = millis();
            if (currentTimeSecond - previousTimeSecond >= intervalNihrom) //if enough time has passed
            {
                previousTimeSecond = currentTimeSecond; //save the last time that the nihrom2 was toggled
                if (!secondNihromActive)               //if not active
                {
                    digitalWrite(nihrom2, HIGH);
                    secondNihromActive = 1;
                    //buzzer::buzz(3400);
                }
                else
                {
                    digitalWrite(nihrom2, LOW);
                    secondNihromActive = 0;
                    iterations++;
                }
            }
        }
    }

    sens_data::BatteryData getBatteryState()
    {
        sens_data::BatteryData BDat;
        BDat.bs = getParachuteBatteryStatus();
        BDat.bat1 = getBattery1Voltage();
        BDat.bat2 = getBattery2Voltage();
        return BDat;
    }

}