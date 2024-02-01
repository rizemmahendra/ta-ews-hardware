#ifndef Network_h
#define Network_h

#include <Arduino.h>
#include <Firebase_ESP_Client.h>
#include <WiFi.h>

class Network
{
private:
    FirebaseAuth _auth;
    FirebaseData _fbdo;
    FirebaseConfig _config;
    String _projectId;
    String _idSungai;

    friend void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info);
    friend void WiFiConnected(WiFiEvent_t event, WiFiEventInfo_t info);

public:
    Network();
    ~Network();
    bool initializeWifi(const char *ssid, const char *password);
    void initializeFirebase(const char *api_key, const char *projectId, const char *email, const char *password, const char *id_sungai);
    void updateDataRealtimeFirebase(FirebaseJson *json, const char *updateMask);
    void updateDataHistoryFirebase(FirebaseJson *json, const char *tanggal, const char *jam);
    bool ready();
};

#endif
