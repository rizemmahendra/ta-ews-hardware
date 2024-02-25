// ===================== LoRa Library & Config =====================
#include <MyLora.h>

const long frequency = 433E6;   // LoRa Frequency
const byte nssPin = 10;         // LoRa radio chip select
const byte resetPin = 9;        // LoRa radio reset
const byte dio0Pin = 2;         // change for your board; must be a hardware interrupt pin
const byte localAddress = 0x01; // address this device
const byte destination = 0x02;  // address destination
unsigned long lastSendTime = 0;
unsigned int interval = 500;

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
DataSensor *reedSwitch = new DataSensor();
// ================================================

MySensor *mySensor = new MySensor();
// =================================================================

// ================== ArduinoJSON Library & Config ==================
#include <ArduinoJson.h>
// =================================================================

void setup()
{
    Serial.begin(9600);

    mySensor->initiliazeWaterLevel(trigPin, echoPin);
    mySensor->initiliazeTurbdidity(ldrPin);
    mySensor->initiliazeRainGauge(reedSwitchPin);

    myLora->initilize(frequency);
}

void loop()
{
    mySensor->getValueWaterLevel(ultrasonik);
    mySensor->getValueTurbdity(ldr);
    mySensor->getValueRainGauge(reedSwitch);
    Serial.println(ultrasonik->value);

    static unsigned long current = 0;

    if (millis() - current > 5000 || current == 0)
    {
        current = millis();
        StaticJsonDocument<128> data;
        data["t"] = ultrasonik->value;
        data["ts"] = ultrasonik->status;
        data["k"] = ldr->value;
        data["ks"] = ldr->status;
        data["h"] = reedSwitch->value;
        data["hs"] = reedSwitch->status;

        serializeJson(data, message);

        unsigned long prev2 = millis();
        myLora->sendMessage(destination, message);
        Serial.print("lama mengirimkan data : ");
        Serial.println(millis() - prev2, DEC);
        Serial.println("mengirim data : " + message);
        Serial.println("-----------------------------------------");
        message = "";
    }
}