#include <Arduino.h>
#include <Network.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>

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
        delay(1000);
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
void Network::initializeFirebase(const char *apiKey, const char *projectId, const char *email, const char *password, const char *idSungai, const char *clientEmail, const char *privateKey)
{
    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
    Firebase.reconnectNetwork(true);

    Network::_config.api_key = apiKey;
    Network::_config.service_account.data.client_email = clientEmail;
    Network::_config.service_account.data.project_id = projectId;
    Network::_config.service_account.data.private_key = privateKey;
    Network::_config.token_status_callback = tokenStatusCallback;
    Network::_config.timeout.serverResponse = 10 * 1000;
    Network::_projectId = projectId;
    Network::_idSungai = idSungai;

    Network::_auth.user.email = email;
    Network::_auth.user.password = password;

    Network::_fbdo.setResponseSize(2048);
    Network::_fbdo.setBSSLBufferSize(4096 /* Rx buffer */, 1024 /* Tx buffer */);

    Serial.println("Begin Firebase");
    Firebase.begin(&_config, &_auth);
}

/**
 * @brief
 * @param gmtOffset_sec
 * @param daylightOffset_sec
 * @param urlNTPServer
 */
void Network::initializeTime(long gmtOffset_sec, int daylightOffset_sec, const char *urlNTPServer)
{
    configTime(gmtOffset_sec, daylightOffset_sec, urlNTPServer);
}

/**
 * @brief
 * @param waktu
 */
void Network::getCurrentTime(Waktu *waktu)
{
    struct tm info;
    if (!getLocalTime(&info))
    {
        Serial.println("Gagal Mendapatkan Waktu Sekarang");
        return;
    }
    strftime(waktu->date, sizeof(waktu->date), "%Y-%m-%d", &info);
    strftime(waktu->time, sizeof(waktu->time), "%H:%M", &info);
    strftime(waktu->hour, sizeof(waktu->hour), "%H", &info);
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
bool Network::updateDataRealtimeFirebase(FirebaseJson *json, const char *updateMask)
{
    FirebaseJson content;

    String documentPath = "TA%20EWS%20RIZEM%20MAHENDRA/" + String(_idSungai) + "/data_sensor/realtime";

    // Serial.print("Update a document... ");

    ESP_LOGW("SEND NOTIFICATION", "Update Data Realtime...");
    if (Firebase.Firestore.patchDocument(&_fbdo, _projectId, "", documentPath.c_str(), json->raw(), updateMask))
    {
        Serial.printf("Ok\n%s\n\n", _fbdo.payload().c_str());
        return true;
    }

    Serial.println(_fbdo.errorReason());
    return false;
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

void Network::sendNotification(const char *title, const char *body, const char *channelId)
{
    FCM_HTTPv1_JSON_Message msg;
    FirebaseJson payload;

    payload.add("title", title);
    payload.add("body", body);
    payload.add("channelId", channelId);

    msg.topic = "ta_ews_rizemmahendra";
    msg.android.priority = "high";
    msg.data = payload.raw();

    // Serial.println("Send Message :  ");
    ESP_LOGW("SEND NOTIFICATION", "Send Notification...");
    if (Firebase.FCM.send(&_fbdo, &msg))
    {
        ESP_LOGI("SEND NOTIFICATION", "Send Notification Successful");
        // Serial.printf("Ok\n%s\n\n", _fbdo.payload());
    }
    else
    {
        ESP_LOGI("SEND NOTIFICATION", "Send Notification Failed");
        Serial.println(_fbdo.errorReason());
    }
}