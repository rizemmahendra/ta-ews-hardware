// ================== LoRa Library & Config ==================
#include <SPI.h>
#include <LoRa.h>

const long frequency = 433E6; // LoRa Frequency
const int nssPin = 5;         // LoRa radio chip select
const int resetPin = 0;       // LoRa radio reset
const int dio0Pin = 2;        // change for your board; must be a hardware interrupt pin
// ============================================================

// ===================== Buzzer Config ========================
const int buzzerPin = 4;
// ============================================================

// ================ Firebase Library & Config ================
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>
#define WIFI_SSID "Base Tekom 19"
#define WIFI_PASSWORD "intelcore19"
#define API_KEY "AIzaSyArrc1cczSPjnnV3MKLRpS_6RksNHjClGw"
#define DATABASE_URL "https://ta-ews-rizem-mahendra-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define USER_EMAIL "gateway@rizemmahendra.com"
#define USER_PASSWORD "Gateway123"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
// ============================================================

void setupLoRA()
{
    Serial.println("LoRa Gateway");
    LoRa.setPins(nssPin, resetPin, dio0Pin);

    if (!LoRa.begin(frequency))
    {
        Serial.println("Starting LoRa failed!");
        while (1)
            ;
    }
}

void setupWifi()
{
    unsigned long ms = millis();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    WiFi.setAutoReconnect(true);
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(1000);
        if (millis() - ms > 10000) // 10 seconds
        {
            return;
        }
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();
}

void setupFirebase()
{
    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
    config.api_key = API_KEY;
    config.database_url = DATABASE_URL;
    config.token_status_callback = tokenStatusCallback;
    config.timeout.serverResponse = 10 * 1000;
    auth.user.email = USER_EMAIL;

    auth.user.password = USER_PASSWORD;
    Firebase.reconnectNetwork(true);
    fbdo.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);

    fbdo.setResponseSize(2048);
    Firebase.begin(&config, &auth);
    while (auth.token.uid == "" || Firebase.isTokenExpired())
    {
        Firebase.refreshToken(&config);
    }
    if (auth.token.uid != "null")
    {
        Serial.print("Connected to Firebase");
        Serial.print(auth.token.uid.c_str());
    }
    else
    {
        Serial.println("Connection to Firebase Failed");
    }
}

void setup()
{
    //  =========== Setup Serial ==========
    Serial.begin(115200);
    while (!Serial)
        ;
    pinMode(buzzerPin, OUTPUT);
    setupLoRA();     // Setup and Inisialization LoRa
    setupWifi();     // Setup and Inisialization Wifi
    setupFirebase(); // Setup and Inisialization Firebase
}

void loop()
{
    // try to parse packet
    int packetSize = LoRa.parsePacket();
    if (packetSize)
    {
        // received a packet
        Serial.print("Received packet '");

        // read packet
        while (LoRa.available())
        {
            Serial.print((char)LoRa.read());
        }

        // print RSSI of packet
        Serial.print("' with RSSI ");
        Serial.println(LoRa.packetRssi());
    }
}