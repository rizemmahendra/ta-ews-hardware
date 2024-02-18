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
void MySensor::initiliazeRainGauge(uint8_t rainGaugePin)
{
    MySensor::_reedSwitchPin = rainGaugePin;
    pinMode(_reedSwitchPin, INPUT);
}

/**
 * @brief get Value of Ultrasonic sensor
 * @return double - distance in cm
 */
double MySensor::getValueWaterLevel()
{
    digitalWrite(_trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(_trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(_trigPin, LOW);

    long duration = pulseIn(_echoPin, HIGH);
    return duration / 58.2; //  58.2 is (speed of sound / 2) in cm/us;
}

/**
 * @brief get Value of LDR MySensor
 * @return double - analog value of LDR
 */
double MySensor::getValueTurbdity()
{
    int value = 0;
    for (int a = 0; a < 10; a++)
    {
        value += analogRead(_ldrPin);
    }
    return value / 10;
}

/**
 * @brief
 * @return
 */
double MySensor::getValueRainGauge()
{
    int tickValue = 10;
    return tickValue * 1.0 /* volume of water*/;
}