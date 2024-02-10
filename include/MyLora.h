#ifndef MyLora_H
#define MyLora_H

#include <LoRa.h>
#include <SPI.h>

class MyLora
{
private:
    uint8_t _nssPin;
    uint8_t _resetPin;
    uint8_t _dio0Pin;
    byte _localAddress;
    String parseData(String dataString);

public:
    MyLora(const byte *nssPin, const byte *resetPin, const byte *dio0Pin, const byte *localAddress);
    ~MyLora();
    void initilize(const long frequency);
    void sendMessage(byte destination, String message);
    String onReceive();
};
#endif