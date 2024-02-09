#ifndef MyLora_H
#define MyLora_H

#include <LoRa.h>
#include <SPI.h>

class MyLora
{
private:
    int _nssPin;
    int _resetPin;
    int _dio0Pin;
    byte _localAddress;
    String parseData(String dataString);

public:
    MyLora(const int *nssPin, const int *resetPin, const int *dio0Pin, const byte *localAddress);
    ~MyLora();
    void initilize(const long frequency);
    void sendMessage(byte destination, String message);
    String onReceive();
};
#endif