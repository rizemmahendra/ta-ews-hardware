#include <Arduino.h>

class DataSensor
{
public:
    float value = 0.0F;
    String status = "";

    DataSensor(const char *defaultStatus, float defaultValue = 0.0F)
    {
        value = defaultValue;
        status = defaultStatus;
    }
};