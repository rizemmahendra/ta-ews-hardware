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
static unsigned long prevSendLora = millis();
static unsigned long intervalGetRainGauge = 60000; // in ms
void printAllData();

void handleReedIntterupt()
{
    static unsigned long prevReed = 0;
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
    mySensor->setLinearRegressionWaterLevel(-0.0165, 14.874);
    mySensor->initiliazeTurbdidity(ldrPin);
    mySensor->setThresholdTurbidity(850, 700);
    mySensor->initiliazeRainGauge(reedSwitchPin, tickVolume, handleReedIntterupt);
    mySensor->setThresholdRainGauge(4, 8);

    Serial.println(F("Node 2 System Begin!"));
    myLora->initilize(frequency);
}

void loop()
{
    message = myLora->onReceive();

    mySensor->getValueWaterLevel(ultrasonik);
    mySensor->getValueTurbdity(ldr);

    current = millis();
    if (current - prevGetRainGauge >= intervalGetRainGauge) // reset tick count every 1menit
    {
        mySensor->getValueRainGauge(reedSwitch);
        mySensor->resetTickCount();
        prevGetRainGauge = current;
    }

    if ((current - prevSend > interval || prevSend == 0) && message != "")
    {
        StaticJsonDocument<126> dataFromNode1;
        deserializeJson(dataFromNode1, message);

        StaticJsonDocument<256> data;
        data["node1"] = dataFromNode1.as<JsonObject>();
        data["node2"]["w"] = ultrasonik->value;
        data["node2"]["ws"] = ultrasonik->status;
        data["node2"]["t"] = ldr->value;
        data["node2"]["ts"] = ldr->status;
        data["node2"]["r"] = reedSwitch->value;
        data["node2"]["rs"] = reedSwitch->status;

        message = "";

        serializeJson(data, message);

        Serial.println(F(""));
        printAllData();
        Serial.println(F(""));

        Serial.println("mengirim data : " + message);
        prevSendLora = millis();
        myLora->sendMessage(destination, message);
        Serial.print(F("lama mengirimkan data : "));
        Serial.print(millis() - prevSendLora, DEC);
        Serial.println(F("ms"));
        Serial.println(F("---------------------------"));

        message = "";

        prevSend = current;
        LoRa.receive();
    }
    delay(100);
}

void printAllData()
{
    Serial.println("## Data Node 2 ##");
    Serial.print("WaterLevel: ");
    Serial.println(ultrasonik->value);
    Serial.print("WaterLevelStatus: ");
    Serial.println(ultrasonik->status);
    Serial.print("WaterTurbidity: ");
    Serial.println(ldr->value);
    Serial.print(F("WaterTurbidityStatus: "));
    Serial.println(ldr->status);
    Serial.print(F("RainIntensisty: "));
    Serial.println(reedSwitch->value);
    Serial.print(F("RainIntensityStatus: "));
    Serial.println(reedSwitch->status);
    Serial.print(F("RainIntensityUpdateIn: "));
    Serial.print(round((intervalGetRainGauge - (current - prevGetRainGauge)) / 1000));
    Serial.println(F("s"));
    Serial.println(F("---------------------------"));
}