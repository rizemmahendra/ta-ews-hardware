// ===================== LoRa Library & Config =====================
#include <MyLora.h>

const long frequency = 433E6;   // LoRa Frequency
const byte nssPin = 10;         // LoRa radio chip select
const byte resetPin = 9;        // LoRa radio reset
const byte dio0Pin = 2;         // change for your board; must be a hardware interrupt pin
const byte localAddress = 0x01; // address this device
const byte destination = 0x02;  // address destination
uint64_t lastSendTime = 0;
uint16_t interval = 5000; // 5000ms or 5s

MyLora *myLora = new MyLora(&nssPin, &resetPin, &dio0Pin, &localAddress);
String message;
// =================================================================

// ==================== Sensor Library & Config ====================
#include <MySensor.h>
// ================== Ultrasonik ==================
const byte trigPin = 4;
const byte echoPin = 5;
DataSensor *ultrasonik = new DataSensor("L");
// ================== Turbidity ===================
const byte ldrPin = A0;
DataSensor *ldr = new DataSensor("CL");
// ================== Rain Gauge ==================
const byte reedSwitchPin = 3;
const float tickVolume = 1;
DataSensor *reedSwitch = new DataSensor("NR");
// ================================================

MySensor *mySensor = new MySensor();
// =================================================================

// ================== ArduinoJSON Library & Config ==================
#include <ArduinoJson.h>
// =================================================================
static uint64_t current;
static uint64_t prevGetRainGauge = 0;
static uint64_t prevSend = 0;
static unsigned long prevSendLora = 0;
static uint32_t sequence = 0;
static unsigned long intervalGetRainGauge = 60000; // in ms
void printAllData();

void handleReedIntterupt()
{
    static unsigned long prevReed = 0;
    // static int count = 0;
    if (millis() - prevReed >= 50) // mencengah double increment akibat bouncing
    {
        mySensor->tickIncreament();
        Serial.println();
        Serial.println(F("Increment Tick"));
        Serial.print("Tick : ");
        Serial.println(mySensor->getTickCount());
        prevReed = millis();
    }
}

void setup()
{
    Serial.begin(9600);
    mySensor->initiliazeWaterLevel(trigPin, echoPin);
    mySensor->setThresholdWaterLevel(4, 7);
    mySensor->setLinearRegressionWaterLevel(-0.0169, 14.817);
    mySensor->initiliazeTurbdidity(ldrPin);
    // mySensor->setThresholdTurbidity(900, 870);
    mySensor->setThresholdTurbidity(750, 550);
    // mySensor->setPolynomialRegressionTurbidity(-0.0001, 0.2152, -156.92, 39269);
    mySensor->initiliazeRainGauge(reedSwitchPin, tickVolume, handleReedIntterupt);
    // mySensor->setThresholdRainGauge(20, 50);
    mySensor->setThresholdRainGauge(4, 8);

    Serial.println(F("Node 1 System Begin!"));
    myLora->initilize(frequency);
}

void loop()
{
    mySensor->getValueTurbdity(ldr);
    mySensor->getValueWaterLevel(ultrasonik);

    current = millis();
    if (current - prevGetRainGauge >= intervalGetRainGauge) // get every 1menit
    {
        mySensor->getValueRainGauge(reedSwitch);
        mySensor->resetTickCount();
        prevGetRainGauge = current;
    }

    if (current - prevSend > interval || prevSend == 0)
    {
        prevSend = current;
        StaticJsonDocument<126> data;
        data["seq"] = sequence++;
        data["w"] = ultrasonik->value;
        data["ws"] = ultrasonik->status;
        data["t"] = ldr->value;
        data["ts"] = ldr->status;
        data["r"] = reedSwitch->value;
        data["rs"] = reedSwitch->status;

        Serial.println(F(""));
        serializeJson(data, message);

        printAllData();
        Serial.println(F(""));
        Serial.println("mengirim data : " + message);
        prevSendLora = millis();
        myLora->sendMessage(destination, message);
        Serial.print("lama transmit data : ");
        Serial.print(millis() - prevSendLora);
        Serial.println(F("ms"));
        Serial.println(F("---------------------------"));
        message = "";
    }
    delay(500);
}

void printAllData()
{
    Serial.println(F("## Data Node 1 ##"));
    Serial.print(F("WaterLevel: "));
    Serial.println(ultrasonik->value);
    Serial.print(F("WaterLevelStatus: "));
    Serial.println(ultrasonik->status);
    Serial.print(F("WaterTurbidity: "));
    Serial.println(ldr->value);
    Serial.print(F("WaterTurbidityStatus: "));
    Serial.println(ldr->status);
    // Serial.print(F("Nilai ADC : "));
    // Serial.println(ldr->value);
    // Serial.print(F("Nilai Volt : "));
    // Serial.println(ldr->value * (5.0 / 1023.0));
    Serial.print(F("RainIntensisty: "));
    Serial.println(reedSwitch->value);
    Serial.print(F("RainIntensityStatus: "));
    Serial.println(reedSwitch->status);
    Serial.print(F("RainIntensityUpdateIn: "));
    Serial.print(round((intervalGetRainGauge - (current - prevGetRainGauge)) / 1000));
    Serial.println(F("s"));
    Serial.println(F("---------------------------"));
}