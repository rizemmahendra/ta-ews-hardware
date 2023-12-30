#include <Arduino.h>
// ===================== LoRa Library & Config =====================
#include <SPI.h>
#include <LoRa.h>

const long frequency = 433E6; // LoRa Frequency
const int nssPin = 10;        // LoRa radio chip select
const int resetPin = 9;       // LoRa radio reset
const int dio0Pin = 2;        // change for your board; must be a hardware interrupt pin
String data;
byte localAddress = 0x02; // address this device
byte destination = 0x03;  // address destination
unsigned long lastSendTime = 0;
unsigned int interval = 2000;
// =================================================================

// ================== ArduinoJSON Library & Config ==================
#include <ArduinoJson.h>
// =================================================================

// ================== Ultrasonik Library & Config ==================
const int trigPin = 4;
const int echoPin = 5;
// =================================================================

// ================== Turbidity Library & Config ==================
const int ldrPin = A0;
// =================================================================

// ================== Rain Gauge Library & Config ==================
const int reedSwitchPin = 3;
// =================================================================

float ultrasonik()
{
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    int durasi = pulseIn(echoPin, HIGH);
    float jarak = (durasi / 2) * 0.034;
    return jarak;
}

float turbidity()
{
    float value = analogRead(ldrPin);
    return value;
}

float rainGauge()
{
    int value = digitalRead(reedSwitchPin);
    return value;
}

void setupLora()
{
    LoRa.setPins(nssPin, resetPin, dio0Pin);
    if (!LoRa.begin(frequency))
    {
        Serial.println("Lora init failed. Check your connections");
        while (true)
            ;
    }
    Serial.println("Lora init Succeeded");
}

void sendMessage(String message)
{
    LoRa.beginPacket();
    LoRa.write(destination);
    LoRa.write(localAddress);
    LoRa.write(message.length());
    LoRa.print(message);
    LoRa.endPacket();
}

String onReceive(int packetSize)
{
    if (packetSize == 0)
        return "";

    byte recipient = LoRa.read();
    byte sender = LoRa.read();
    byte incomingLength = LoRa.read();
    // received a packet
    String incoming = "";

    if (recipient != localAddress)
    {
        Serial.println("This Message is Not For Me!");
        return "";
    }
    // read packet
    while (LoRa.available())
    {
        incoming += (char)LoRa.read();
    }

    if (incoming.length() != incomingLength)
    {
        Serial.println("Message lenght does not match lenght");
        return "";
    }

    // print RSSI of packet
    // Serial.println((String)incoming);
    // Serial.print("' with RSSI ");
    // Serial.println(LoRa.packetRssi());
    // Serial.println("Received from: 0x" + String(sender, HEX));
    // Serial.println("Sent to: 0x" + String(recipient, HEX));
    // Serial.println("Message length: " + String(incomingLength));
    // Serial.println("Message: " + incoming);
    // Serial.println("RSSI: " + String(LoRa.packetRssi()));
    // Serial.println("Snr: " + String(LoRa.packetSnr()));
    // Serial.println();
    return incoming;
}

void setup()
{
    Serial.begin(9600);
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
    pinMode(ldrPin, INPUT);
    pinMode(reedSwitchPin, INPUT);
    setupLora();
}
void loop()
{
    data = onReceive(LoRa.parsePacket());
    if (data != "")
    {
        Serial.println(data);
    }
    StaticJsonDocument<256> dataNode;
    // dataNode["jarak"] = ultrasonik();
    dataNode["kekeruhan"] = turbidity();
    dataNode["hujan"] = rainGauge();
    String message;
    serializeJson(dataNode, message);

    if (data != "")
    {
        Serial.println("=====================================");
        Serial.println("incoming : " + data);
        Serial.println("me : " + message);

        StaticJsonDocument<512> allData;
        allData["node1"] = data;
        allData["node2"] = message;
        String allmessage;
        serializeJson(allData, allmessage);

        Serial.println("all : " + allmessage);
        Serial.println("=====================================");
    }
    if (millis() - lastSendTime > interval)
    {
        sendMessage(message);
        Serial.println("mengirim data : " + message);
        lastSendTime = millis();
    }
}