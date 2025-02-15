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
    uint8_t _waterLevelLowThreshold;
    uint8_t _waterLevelMediumThreshold;
    bool _linearRegressionWaterLevel = false;
    float _valueAWaterLevel;
    float _valueBWaterLevel;

    // Turbidity or LDR
    uint8_t _ldrPin;
    uint16_t _turbidityClearThreshold;
    uint16_t _turbidityMurkyThreshold;
    bool _polynomialRegressionTurbidity = false;
    float _valueATurbidity;
    float _valueBTurbidity;
    float _valueCTurbidity;
    float _valueDTurbidity;

    // Rain Gauge or ReedSwitch
    uint8_t _reedSwitchPin;
    uint8_t _tickValue = 0;
    float _tickVolume;
    uint8_t _noRainThreshold = 0;
    uint8_t _lightRainThreshold;
    uint8_t _moderateRainThreshold;

public:
    MySensor();
    ~MySensor();
    void initiliazeWaterLevel(uint8_t trigPin, uint8_t echoPin);
    void setThresholdWaterLevel(uint8_t lowThreshold, uint8_t mediumThreshold);
    void setLinearRegressionWaterLevel(float a, float b);
    void initiliazeTurbdidity(uint8_t turbidityPin);
    void setThresholdTurbidity(uint16_t clearThreshold, uint16_t murkyThreshold);
    void setPolynomialRegressionTurbidity(float a, float b, float c, float d);
    void initiliazeRainGauge(uint8_t rainGaugePin, float tickVolume, void (*callback)(void));
    void setThresholdRainGauge(uint8_t lightThreshold, uint8_t moderateThreshold);
    void getValueWaterLevel(DataSensor *sensor);
    void getValueTurbdity(DataSensor *sensor);
    void getValueRainGauge(DataSensor *sensor);
    void tickIncreament();
    void resetTickCount();
    uint8_t getTickCount();
};

#endif
