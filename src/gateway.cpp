// ================== LoRa Library & Config ==================
#include <SPI.h>
#include <LoRa.h>

const long frequency = 433E6; // LoRa Frequency
const int nssPin = 5;         // LoRa radio chip select
const int resetPin = 0;       // LoRa radio reset
const int dio0Pin = 2;        // change for your board; must be a hardware interrupt pin
String data;
byte localAddress = 0x02; // address this device
byte destination = 0x01;  // address destination
unsigned long lastSendTime = 0;
unsigned int interval = 2000;
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
    Serial.println("LoRa init Successfull");
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
        Serial.print("Connected to Firebase with id: ");
        Serial.println(auth.token.uid.c_str());
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

String onReceive(int packetSize)
{
    if (packetSize == 0)
        return "";

    byte recipient = LoRa.read();
    byte sender = LoRa.read();
    byte incomingLength = LoRa.read();
    // received a packet
    String incoming = "";

    if (recipient != localAddress)
    {
        Serial.println("This Message is Not For Me!");
        return "";
    }
    // read packet
    while (LoRa.available())
    {
        incoming += (char)LoRa.read();
    }

    if (incoming.length() != incomingLength)
    {
        Serial.println("Message lenght does not match lenght");
        return "";
    }

    // print RSSI of packet
    // Serial.println((String)incoming);
    // Serial.print("' with RSSI ");
    // Serial.println(LoRa.packetRssi());
    Serial.println("Received from: 0x" + String(sender, HEX));
    Serial.println("Sent to: 0x" + String(recipient, HEX));
    Serial.println("Message length: " + String(incomingLength));
    Serial.println("Message: " + incoming);
    Serial.println("RSSI: " + String(LoRa.packetRssi()));
    Serial.println("Snr: " + String(LoRa.packetSnr()));
    Serial.println();
    return incoming;
}

void loop()
{
    // try to parse packet
    // onReceive(LoRa.parsePacket());
    data = onReceive(LoRa.parsePacket());
    // Serial.println(data.length());
    if (Firebase.ready() && data != "")
    {
        Serial.printf("Set Ketinggian Air: %.2f ... %s\n", data.toFloat(), Firebase.RTDB.setFloat(&fbdo, "/jarak", data.toFloat()) ? "Ok" : fbdo.errorReason().c_str());
        Serial.printf("Set timestamp... %s\n", Firebase.RTDB.setTimestamp(&fbdo, "/timestamp") ? "ok" : fbdo.errorReason().c_str());
    }
}