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
 * @brief Set Linear Regression for water level measurement, format y = ax + b
 * @param a value a in linear regression formula
 * @param b value b in linear regression formula
 */
void MySensor::setLinearRegressionWaterLevel(float a, float b)
{
    MySensor::_linearRegressionWaterLevel = true;
    MySensor::_valueAWaterLevel = a;
    MySensor::_valueBWaterLevel = b;
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
 * @brief Set Polynomial Regression for turbidity measurement, format y = ax^3 + bx^2 + cx + d
 * @param a value a in polynomial regression formula
 * @param b value b in polynomial regression formula
 * @param c value c in polynomial regression formula
 * @param d value d in polynomial regression formula
 */
void MySensor::setPolynomialRegressionTurbidity(float a, float b, float c, float d)
{
    MySensor::_polynomialRegressionTurbidity = true;
    MySensor::_valueATurbidity = a;
    MySensor::_valueBTurbidity = b;
    MySensor::_valueCTurbidity = c;
    MySensor::_valueDTurbidity = d;
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
    float distance = 0.0F;
    // float distance = duration / 58.2; //  58.2 is (speed of sound / 2) in cm/us;
    if (MySensor::_linearRegressionWaterLevel)
    {
        distance = (MySensor::_valueAWaterLevel * duration) + MySensor::_valueBWaterLevel;
    }
    else
    {
        distance = (duration / 2) * 0.0343;
    }
    sensor->value = (roundf(distance * 100)) / 100.0;
    sensor->value = sensor->value < 0 ? 0 : sensor->value;
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
    // Serial.println();
    // Serial.print("Nilai ADC : ");
    // Serial.println(value);
    // Serial.print("Nilai V : ");
    // Serial.println((value / 1024) * 5);
    // Serial.print("Nilai Keruh 1 : ");
    // Serial.println(-733.297808903065 * value + 3345.169597143254);
    // Serial.print("Nilai Keruh 2 : ");
    // Serial.println(1000.00 - (((value / 1024) * 5) / 4.41) * 1000.00);
    if (MySensor::_polynomialRegressionTurbidity)
    {
        sensor->value = MySensor::_valueATurbidity * pow(value, 3) + MySensor::_valueBTurbidity * pow(value, 2) + MySensor::_valueCTurbidity * value + MySensor::_valueDTurbidity;
    }
    else
    {
        sensor->value = value;
    }

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
uint8_t MySensor::getTickCount()
{
    return MySensor::_tickValue;
}

void MySensor::resetTickCount()
{
    MySensor::_tickValue = 0;
}