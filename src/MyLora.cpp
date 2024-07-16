#include <MyLora.h>

MyLora::MyLora(const byte *nssPin, const byte *resetPin, const byte *dio0Pin, const byte *localAddress)
{
    MyLora::_resetPin = *resetPin;
    MyLora::_nssPin = *nssPin;
    MyLora::_dio0Pin = *dio0Pin;
    MyLora::_localAddress = *localAddress;
}

MyLora::~MyLora() {}

void MyLora::initilize(const long frequency)
{
    LoRa.setPins(_nssPin, _resetPin, _dio0Pin);
    if (!LoRa.begin(frequency))
    {
        Serial.println(F("Lora init failed. Check your connections"));
        while (true)
            ;
    }
    // LoRa.setSignalBandwidth(10.4E3);
    LoRa.setTxPower(20);
    Serial.println(F("Lora init Succeeded"));
}

void MyLora::sendMessage(byte destination, String message)
{
    LoRa.beginPacket();
    LoRa.write(destination);
    LoRa.write(_localAddress);
    LoRa.write(message.length());
    LoRa.print(message);
    LoRa.endPacket();
}

String MyLora::onReceive()
{
    if (LoRa.parsePacket() == 0)
        return "";

    byte recipient = LoRa.read();
    byte sender = LoRa.read();
    byte incomingLength = LoRa.read();
    // received a packet
    String incoming = "";

    if (recipient != _localAddress)
    {
        Serial.println(F("This Message is Not For Me!"));
        return "";
    }
    // read packet
    while (LoRa.available())
    {
        incoming += (char)LoRa.read();
    }

    if (incoming.length() != incomingLength)
    {
        Serial.println(F("Message lenght does not match lenght"));
        return "";
    }
    Serial.println("");
    Serial.print(F("From:"));
    Serial.println(sender, HEX);
    Serial.print(F("Rssi:"));
    Serial.println(LoRa.packetRssi());
    return incoming;
}

String MyLora::parseData(String dataString)
{
    return String();
}