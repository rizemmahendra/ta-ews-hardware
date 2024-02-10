#ifndef Sensor_H
#define Sensor_H

#include <Arduino.h>

class Sensor
{
private:
    // Ultrasonik
    uint8_t _trigPin;
    uint8_t _echoPin;

    // Turbidity or LDR
    uint8_t _ldrPin;

    // Rain Gauge or ReedSwitch
    uint8_t _reedSwitchPin;

public:
    Sensor();
    ~Sensor();
    void initiliazeWaterLevel(uint8_t trigPin, uint8_t echoPin);
    void initiliazeTurbdidity(uint8_t turbidityPin);
    void initiliazeRainGauge(uint8_t rainGaugePin);
    double getValueWaterLevel();
    double getValueTurbdity();
    double getValueRainGauge();
};

#endif