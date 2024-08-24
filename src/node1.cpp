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
const float tickVolume = 1.4;
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

void handleReedIntterupt()
{
    static unsigned long prevReed = 0;
    static int count = 0;
    if (millis() - prevReed >= 50) // mencengah double increment akibat bouncing
    {
        mySensor->tickIncreament();
        Serial.println(F("Increment Tick"));
        Serial.print("Tick : ");
        Serial.println(++count);
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
    mySensor->setThresholdTurbidity(950, 800);
    mySensor->initiliazeRainGauge(reedSwitchPin, tickVolume, handleReedIntterupt);
    mySensor->setThresholdRainGauge(4, 8);

    myLora->initilize(frequency);
}

void loop()
{
    mySensor->getValueWaterLevel(ultrasonik);
    mySensor->getValueTurbdity(ldr);
    mySensor->getValueRainGauge(reedSwitch);

    current = millis();
    if (current - prevGetRainGauge >= 60000) // get every 1menit
    {
        mySensor->resetTickCount();
        prevGetRainGauge = current;
    }

    if (current - prevSend > interval || prevSend == 0)
    {
        prevSend = current;
        StaticJsonDocument<126> data;
        data["w"] = ultrasonik->value;
        data["ws"] = ultrasonik->status;
        data["t"] = ldr->value;
        data["ts"] = ldr->status;
        data["r"] = reedSwitch->value;
        data["rs"] = reedSwitch->status;

        serializeJson(data, message);

        prevSendLora = millis();
        myLora->sendMessage(destination, message);
        Serial.println("mengirim data : " + message);
        Serial.print("lama transmit data : ");
        Serial.println(millis() - prevSendLora);
        Serial.println(F(""));
        message = "";
    }
    delay(500);
}