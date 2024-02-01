#include <Arduino.h>
#include <Network.h>

#define WIFI_SSID "Base Tekom 19"
#define WIFI_PASSWORD "kadrunireng"
// #define WIFI_SSID "RizemMahendra"
// #define WIFI_PASSWORD "terserahse"

#define API_KEY "AIzaSyArrc1cczSPjnnV3MKLRpS_6RksNHjClGw"
#define USER_EMAIL "gateway@rizemmahendra.com"
#define USER_PASSWORD "Gateway123"
#define FIREBASE_PROJECT_ID "ta-ews-rizem-mahendra"
#define ID_SUNGAI "axBPVZsdXUAjFyWOlXnt"

#include "esp_sntp.h"

Network *connection = new Network();

void setup()
{
    Serial.begin(115200);
    if (connection->initializeWifi(WIFI_SSID, WIFI_PASSWORD))
    {
        connection->initializeFirebase(API_KEY, FIREBASE_PROJECT_ID, USER_EMAIL, USER_PASSWORD, ID_SUNGAI);
    }

    FirebaseJson content;
    String updateMask = "";
    // Node 1
    content.set("fields/node1/mapValue/fields/levelDanger/stringValue", "aman");
    content.set("fields/node1/mapValue/fields/waterLevel/doubleValue", 10);
    content.set("fields/node1/mapValue/fields/waterLevelStatus/stringValue", "rendah");
    content.set("fields/node1/mapValue/fields/waterTurbidity/doubleValue", 20);
    content.set("fields/node1/mapValue/fields/waterTurbidityStatus/stringValue", "tidak keruh");
    content.set("fields/node1/mapValue/fields/rainIntensity/doubleValue", 30);
    content.set("fields/node1/mapValue/fields/rainIntensityStatus/stringValue", "tidak hujan");
    updateMask += "node1.levelDanger, node1.waterLevel, node1.waterLevelStatus, node1.waterTurbidity, node1.waterTurbidityStatus, node1.rainIntensity, node1.rainIntensityStatus,";

    // Node 2
    content.set("fields/node2/mapValue/fields/levelDanger/stringValue", "aman");
    content.set("fields/node2/mapValue/fields/waterLevel/doubleValue", 40);
    content.set("fields/node2/mapValue/fields/waterLevelStatus/stringValue", "sedang");
    content.set("fields/node2/mapValue/fields/waterTurbidity/doubleValue", 50);
    content.set("fields/node2/mapValue/fields/waterTurbidityStatus/stringValue", "keruh");
    content.set("fields/node2/mapValue/fields/rainIntensity/doubleValue", 60);
    content.set("fields/node2/mapValue/fields/rainIntensityStatus/stringValue", "hujan");
    updateMask += "node2.levelDanger, node2.waterLevel, node2.waterLevelStatus, node2.waterTurbidity, node2.waterTurbidityStatus, node2.rainIntensity, node2.rainIntensityStatus,";
    String tanggal = "2024-02-01";
    String jam = "01:00";
    delay(2000);
    if (connection->ready())
    {
        connection->updateDataRealtimeFirebase(&content, updateMask.c_str());
        connection->updateDataHistoryFirebase(&content, tanggal.c_str(), jam.c_str());
    }
}

void loop()
{
    // if (connection->ready())
    // {
    // connection->updateDataRealtimeFirebase(&content, updateMask.c_str());
    // }
}