#include <Arduino.h>
#include <Network.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

Network::Network()
{
}

Network::~Network()
{
}

void WiFiConnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
    Serial.println("WiFi Conneceted");
}

void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info)
{
    Serial.print("IP Address : ");
    Serial.println(WiFi.localIP());
}

/**
 * @brief
 * @param ssid Name of wifi
 * @param password password wifi
 * @return bool
 */
bool Network::initializeWifi(const char *ssid, const char *password)
{
    WiFi.onEvent(WiFiConnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_CONNECTED);
    WiFi.onEvent(WiFiGotIP, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
    WiFi.setAutoReconnect(true);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
    }
    return true;
}

/**
 * @brief
 * @param api_key Api key project firebase
 * @param projectId Project Id Firebase
 * @param email Email for authentication
 * @param password Passowrd for authentication
 * @param id_sungai Id Document sungai di firebase
 * @note email and password required if anonymous authentication not active
 */
void Network::initializeFirebase(const char *api_key, const char *projectId, const char *email, const char *password, const char *id_sungai)
{
    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
    Firebase.reconnectNetwork(true);

    Network::_config.api_key = api_key;
    Network::_config.token_status_callback = tokenStatusCallback;
    Network::_config.timeout.serverResponse = 10 * 1000;
    Network::_projectId = projectId;
    Network::_idSungai = id_sungai;

    Network::_auth.user.email = email;
    Network::_auth.user.password = password;

    Network::_fbdo.setResponseSize(2048);
    Network::_fbdo.setBSSLBufferSize(4096 /* Rx buffer */, 1024 /* Tx buffer */);

    Serial.println("Begin Firebase");
    Firebase.begin(&_config, &_auth);
}

/**
 * @brief
 * @return Boolean, true is connection wifi and firebase ready
 */
bool Network::ready()
{
    return (WiFi.status() == WL_CONNECTED && Firebase.ready());
}

/**
 * @brief Funtion to update data firebase
 * @param json Pointer to FirebaseJson Object
 * @param updateMask Pointer to String update mask
 */
void Network::updateDataRealtimeFirebase(FirebaseJson *json, const char *updateMask)
{
    FirebaseJson content;

    String documentPath = "TA%20EWS%20RIZEM%20MAHENDRA/" + String(_idSungai) + "/data_sensor/realtime";

    Serial.print("Update a document... ");

    if (Firebase.Firestore.patchDocument(&_fbdo, _projectId, "", documentPath.c_str(), json->raw(), updateMask))
    {
        Serial.printf("ok\n%s\n\n", _fbdo.payload().c_str());
    }
    else
    {
        Serial.println(_fbdo.errorReason());
    }
}

/**
 * @brief
 * @param json Pointer to FirebaseJson Object
 * @param tanggal Tanggal yyyy-mm-dd
 * @param jam Jam hh:mm
 */
void Network::updateDataHistoryFirebase(FirebaseJson *json, const char *tanggal, const char *jam)
{
    String documentPath = "TA%20EWS%20RIZEM%20MAHENDRA/" + String(_idSungai) + "/data_sensor/history/" + String(tanggal) + "/" + String(jam);

    Serial.println("Create a document ... ");
    if (Firebase.Firestore.createDocument(&_fbdo, _projectId, "", documentPath.c_str(), json->raw()))
    {
        Serial.printf("ok\n%s\n\n", _fbdo.payload().c_str());
    }
    else
    {
        Serial.println(_fbdo.errorReason());
    }
}
