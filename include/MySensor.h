#ifndef MySensor_H
#define MySensor_H

#include <Arduino.h>
#include <DataSensor.cpp>

class MySensor
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
    MySensor();
    ~MySensor();
    void initiliazeWaterLevel(uint8_t trigPin, uint8_t echoPin);
    void initiliazeTurbdidity(uint8_t turbidityPin);
    void initiliazeRainGauge(uint8_t rainGaugePin);
    void getValueWaterLevel(DataSensor *sensor);
    void getValueTurbdity(DataSensor *sensor);
    void getValueRainGauge(DataSensor *sensor);
};

#endif