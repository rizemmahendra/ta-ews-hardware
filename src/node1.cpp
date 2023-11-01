#include <Arduino.h>

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

void setup()
{
    Serial.begin(9600);
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
}
void loop()
{
    Serial.print("jarak : ");
    Serial.println(ultrasonik());
    delay(1000);
}