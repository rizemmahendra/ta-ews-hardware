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
 * @param turbidityPin
 */
void MySensor::initiliazeTurbdidity(uint8_t turbidityPin)
{
    MySensor::_ldrPin = turbidityPin;
    pinMode(_ldrPin, INPUT);
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

    if (distance <= 5)
    {
        sensor->status = "L";
    }
    else if (distance <= 10)
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
    int value = 0;
    for (int a = 0; a < 10; a++)
    {
        value += analogRead(_ldrPin);
    }
    sensor->value = value / 10;
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
    sensor->value = MySensor::_tickValue * MySensor::_tickVolume;
}
void MySensor::resetTickCount()
{
    MySensor::_tickValue = 0;
}