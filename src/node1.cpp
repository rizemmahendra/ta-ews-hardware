#include <Arduino.h>
// ===================== LoRa Library & Config =====================
#include <SPI.h>
#include <LoRa.h>

const long frequency = 433E6; // LoRa Frequency
const int nssPin = 10;        // LoRa radio chip select
const int resetPin = 9;       // LoRa radio reset
const int dio0Pin = 2;        // change for your board; must be a hardware interrupt pin
String data;
byte localAddress = 0x01; // address this device
byte destination = 0x02;  // address destination
unsigned long lastSendTime = 0;
unsigned int interval = 2000;
// =================================================================

// ================== Ultrasonik Library & Config ==================
const int trigPin = 4;
const int echoPin = 5;
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

void setup()
{
    Serial.begin(9600);
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
    setupLora();
}
void loop()
{
    float jarak = ultrasonik();
    data = (String)jarak;

    delay(1000);
    if (millis() - lastSendTime > interval)
    {
        // String message = (String)jarak;
        sendMessage(data);
        Serial.println("mengirim data : " + data);
        lastSendTime = millis();
    }
}
