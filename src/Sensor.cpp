#include <Sensor.h>

Sensor::Sensor(/* args */)
{
}

Sensor::~Sensor()
{
}

/**
 * @brief
 * @param trigPin
 * @param echoPin
 */
void Sensor::initiliazeWaterLevel(uint8_t trigPin, uint8_t echoPin)
{
    Sensor::_trigPin = trigPin;
    Sensor::_echoPin = echoPin;
    pinMode(_trigPin, OUTPUT);
    pinMode(_echoPin, INPUT);
}

/**
 * @brief
 * @param turbidityPin
 */
void Sensor::initiliazeTurbdidity(uint8_t turbidityPin)
{
    Sensor::_ldrPin = turbidityPin;
    pinMode(_ldrPin, INPUT);
}

/**
 * @brief
 * @param rainGaugePin
 */
void Sensor::initiliazeRainGauge(uint8_t rainGaugePin)
{
    Sensor::_reedSwitchPin = rainGaugePin;
    pinMode(_reedSwitchPin, INPUT);
}

/**
 * @brief get Value of Ultrasonic sensor
 * @return double - distance in cm
 */
double Sensor::getValueWaterLevel()
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
 * @brief get Value of LDR Sensor
 * @return double - analog value of LDR
 */
double Sensor::getValueTurbdity()
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
double Sensor::getValueRainGauge()
{
    int tickValue = 10;
    return tickValue * 1.0 /* volume of water*/;
}