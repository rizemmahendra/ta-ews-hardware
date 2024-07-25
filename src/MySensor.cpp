#include <MySensor.h>

MySensor::MySensor(/* args */)
{
}

MySensor::~MySensor()
{
}

/**
 * @brief
 * @param trigPin
 * @param echoPin
 */
void MySensor::initiliazeWaterLevel(uint8_t trigPin, uint8_t echoPin)
{
    MySensor::_trigPin = trigPin;
    MySensor::_echoPin = echoPin;
    pinMode(_trigPin, OUTPUT);
    pinMode(_echoPin, INPUT);
    digitalWrite(_trigPin, LOW);
}
/**
 * @brief
 * @param lowThreshold maximum value to enter "low" status, status set to "low" if value smaller than `lowTreshold` value
 * @param mediumThreshold maximum value to enter "medium" status, status set to "medium" if value smaller than `mediumTreshold` value
 * @note `highThreshold` automatic set, status set to "high" if value greater than `mediumTreshold` value
 */
void MySensor::setThresholdWaterLevel(uint8_t lowThreshold, uint8_t mediumThreshold)
{
    MySensor::_waterLevelLowThreshold = lowThreshold;
    MySensor::_waterLevelMediumThreshold = mediumThreshold;
}

/**
 * @brief
 * @param turbidityPin
 */
void MySensor::initiliazeTurbdidity(uint8_t turbidityPin)
{
    MySensor::_ldrPin = turbidityPin;
    pinMode(_ldrPin, INPUT);
}
/**
 * @brief
 * @param clearThreshold minimum value to enter "clear" status, status set to "clear" if value greater than `clearTreshold` value
 * @param murkyThreshold minimum value to enter "murky" status, status set to "murky" if value greater than `lowTreshold` value
 * @note `turbidThreshold` automatic set, status set to "turbid" if value smaller than `murkyTreshold` value
 */
void MySensor::setThresholdTurbidity(uint16_t clearThreshold, uint16_t murkyThreshold)
{
    MySensor::_turbidityClearThreshold = clearThreshold;
    MySensor::_turbidityMurkyThreshold = murkyThreshold;
}

/**
 * @brief
 * @param rainGaugePin
 */
void MySensor::initiliazeRainGauge(uint8_t rainGaugePin, float tickVolume, void (*callback)(void))
{
    MySensor::_reedSwitchPin = rainGaugePin;
    MySensor::_tickVolume = tickVolume;
    pinMode(_reedSwitchPin, INPUT);
    attachInterrupt(digitalPinToInterrupt(_reedSwitchPin), callback, FALLING);
}
/**
 * @brief
 * @param lightThreshold maximum value to enter "light" status, status set to "light" if value smaller than lightTreshold value
 * @param moderateThreshold maximum value to enter "moderate" status, status set to "moderate" if value smaller than moderateTreshold value
 * @note `noRainThreshold` automatic set to 0, status set to "noRain" if value same with 0
 * @note `HeavyThreshold` automatic set, status set to "heavy" if value greater than `moderateTreshold` value
 */
void MySensor::setThresholdRainGauge(uint8_t lightThreshold, uint8_t moderateThreshold)
{
    MySensor::_lightRainThreshold = lightThreshold;
    MySensor::_moderateRainThreshold = moderateThreshold;
}

/**
 * @brief get Value of Ultrasonic sensor
 * @return double - distance in cm
 */
void MySensor::getValueWaterLevel(DataSensor *sensor)
{
    delayMicroseconds(2);
    digitalWrite(_trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(_trigPin, LOW);

    long duration = pulseIn(_echoPin, HIGH);
    // float distance = duration / 58.2; //  58.2 is (speed of sound / 2) in cm/us;
    float distance = (duration / 2) * 0.0343;
    sensor->value = distance;
    /**
     * Tinggi Air : L (Low), M (Medium), H (High)
     */
    if (distance <= MySensor::_waterLevelLowThreshold)
    {
        sensor->status = "L";
    }
    else if (distance <= MySensor::_waterLevelMediumThreshold)
    {
        sensor->status = "M";
    }
    else
    {
        sensor->status = "H";
    }
}

/**
 * @brief get Value of LDR MySensor
 * @return double - analog value of LDR
 */
void MySensor::getValueTurbdity(DataSensor *sensor)
{
    float value = 0;
    for (int a = 0; a < 10; a++)
    {
        value += analogRead(_ldrPin);
    }
    value = value / 10;
    sensor->value = value;
    /**
     * CL (Clear),M (Murky),T (Turbid)
     */
    if (value > MySensor::_turbidityClearThreshold)
    {
        sensor->status = "CL";
    }
    else if (value > MySensor::_turbidityMurkyThreshold)
    {
        sensor->status = "M";
    }
    else
    {
        sensor->status = "T";
    }
}

void MySensor::tickIncreament()
{
    MySensor::_tickValue++;
}
/**
 * @brief
 * @return
 */
void MySensor::getValueRainGauge(DataSensor *sensor)
{
    float value = 0;
    value = MySensor::_tickValue * MySensor::_tickVolume;
    sensor->value = value;
    /**
     * NR (No Rain),L (Light), M (Moderate), H (Heavy)
     */
    if (value == MySensor::_noRainThreshold)
    {
        sensor->status = "NR";
    }
    else if (value < MySensor::_lightRainThreshold)
    {
        sensor->status = "L";
    }
    else if (value < MySensor::_moderateRainThreshold)
    {
        sensor->status = "M";
    }
    else
    {
        sensor->status = "H";
    }
}
void MySensor::resetTickCount()
{
    MySensor::_tickValue = 0;
}