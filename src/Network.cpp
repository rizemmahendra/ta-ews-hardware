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
 * @param database_url Url database
 * @param email Email for authentication
 * @param password Passowrd for authentication
 * @note email and password required if anonymous authentication not active
 */
void Network::initializeFirebase(const char *api_key, const char *database_url, const char *email, const char *password)
{
    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
    Firebase.reconnectNetwork(true);

    Network::_config.api_key = api_key;
    Network::_config.database_url = database_url;
    Network::_config.token_status_callback = tokenStatusCallback;
    Network::_config.timeout.serverResponse = 10 * 1000;

    Network::_auth.user.email = email;
    Network::_auth.user.password = password;

    Network::_fbdo.setResponseSize(2048);
    Network::_fbdo.setBSSLBufferSize(4096 /* Rx buffer */, 1024 /* Tx buffer */);

    Serial.println("Begin Firebase");
    Firebase.begin(&_config, &_auth);
}

/**
 * @brief
 * @param json Pointer to FirebaseJson Object
 */
void Network::updateDataFirebase(FirebaseJson *json)
{
    if (Firebase.RTDB.setJSON(&_fbdo, "/data", json))
    {
        Serial.println("Success");
    }
    else
    {
        Serial.println(_fbdo.errorReason().c_str());
    }
}

/**
 * @brief
 * @return boolean, true is connection wifi and firebase ready
 */
bool Network::ready()
{
    return (WiFi.status() == WL_CONNECTED && Firebase.ready());
}