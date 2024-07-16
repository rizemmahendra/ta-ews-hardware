// ===================== LoRa Library & Config =====================
#include <MyLora.h>

const long frequency = 433E6;   // LoRa Frequency
const byte nssPin = 10;         // LoRa radio chip select
const byte resetPin = 9;        // LoRa radio reset
const byte dio0Pin = 2;         // change for your board; must be a hardware interrupt pin
const byte localAddress = 0x02; // address this device
const byte destination = 0xFF;  // address destination
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
DataSensor *ultrasonik = new DataSensor();
// ================== Turbidity ===================
const byte ldrPin = A0;
DataSensor *ldr = new DataSensor();
// ================== Rain Gauge ==================
const byte reedSwitchPin = 3;
const float tickVolume = 1.4;
DataSensor *reedSwitch = new DataSensor();
// ================================================

MySensor *mySensor = new MySensor();
// =================================================================

// ================== ArduinoJSON Library & Config ==================
#include <ArduinoJson.h>
// =================================================================

void handleReedIntterupt()
{
    static unsigned long prevReed = 0;
    if (millis() - prevReed >= 50) // mencengah double increment akibat bouncing
    {
        Serial.println(prevReed);
        mySensor->tickIncreament();
        Serial.println(F("Increment Tick"));
        prevReed = millis();
    }
}

void setup()
{
    Serial.begin(9600);

    mySensor->initiliazeWaterLevel(trigPin, echoPin);
    mySensor->initiliazeTurbdidity(ldrPin);
    mySensor->initiliazeRainGauge(reedSwitchPin, tickVolume, handleReedIntterupt);
    myLora->initilize(frequency);
}

void loop()
{
    message = myLora->onReceive();

    mySensor->getValueWaterLevel(ultrasonik);
    mySensor->getValueTurbdity(ldr);
    mySensor->getValueRainGauge(reedSwitch);

    static uint64_t prevGetRainGauge = 0;
    if (millis() - prevGetRainGauge >= 60000) // reset tick count every 1menit
    {
        mySensor->resetTickCount();
        prevGetRainGauge = millis();
    }

    static uint64_t prevSend = 0;
    if (millis() - prevSend > interval && message != "")
    {
        Serial.println(message);
        StaticJsonDocument<128> dataFromNode1;
        deserializeJson(dataFromNode1, message);

        StaticJsonDocument<256> data;
        data["node1"] = dataFromNode1.as<JsonObject>();
        data["node2"]["t"] = ultrasonik->value;
        data["node2"]["ts"] = ultrasonik->status;
        data["node2"]["k"] = ldr->value;
        data["node2"]["ks"] = ldr->status;
        data["node2"]["h"] = reedSwitch->value;
        data["node2"]["hs"] = reedSwitch->status;

        message = "";
        serializeJson(data, message);

        unsigned long prev2 = millis();
        myLora->sendMessage(destination, message);
        Serial.print(F("lama mengirimkan data : "));
        Serial.println(millis() - prev2, DEC);
        Serial.println("mengirim data : " + message);
        Serial.println(F("-----------------------------------------"));
        message = "";
        LoRa.receive();
    }
}