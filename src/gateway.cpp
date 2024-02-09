#include <Arduino.h>
// ===================== LoRa Library & Config =====================
#include <MyLora.h>

const long frequency = 433E6; // LoRa Frequency
const int nssPin = 5;         // LoRa radio chip select
const int resetPin = 14;      // LoRa radio reset
const int dio0Pin = 2;        // change for your board; must be a hardware interrupt pin
// String data;
byte localAddress = 0xFF; // address this device
MyLora *myLora = new MyLora(&nssPin, &resetPin, &dio0Pin, &localAddress);
// =================================================================
// #include <Network.h>

// #define WIFI_SSID "Base Tekom 19"
// #define WIFI_PASSWORD "kadrunireng"
// // #define WIFI_SSID "RizemMahendra"
// // #define WIFI_PASSWORD "terserahse"

// #define API_KEY "AIzaSyArrc1cczSPjnnV3MKLRpS_6RksNHjClGw"
// #define USER_EMAIL "gateway@rizemmahendra.com"
// #define USER_PASSWORD "Gateway123"
// #define FIREBASE_PROJECT_ID "ta-ews-rizem-mahendra"
// #define ID_SUNGAI "axBPVZsdXUAjFyWOlXnt"

// Network *connection = new Network();
// Waktu *waktu = new Waktu();
// const char *ntpServer = "id.pool.ntp.org";
// const long gmtOffset_sec = 7 * 3600; // GMT+7 in seconds
// const int daylightOffset_sec = 0;

void setup()
{
    Serial.begin(115200);
    myLora->initilize(frequency);
    String data = myLora->onReceive();
    if (data != "")
    {
        Serial.println(data);
    }
    // if (connection->initializeWifi(WIFI_SSID, WIFI_PASSWORD))
    // {
    //     connection->initializeTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    //     // connection->initializeFirebase(API_KEY, FIREBASE_PROJECT_ID, USER_EMAIL, USER_PASSWORD, ID_SUNGAI);
    // }

    // FirebaseJson content;
    // String updateMask = "";
    // // Node 1
    // content.set("fields/node1/mapValue/fields/levelDanger/stringValue", "aman");
    // content.set("fields/node1/mapValue/fields/waterLevel/doubleValue", 10);
    // content.set("fields/node1/mapValue/fields/waterLevelStatus/stringValue", "rendah");
    // content.set("fields/node1/mapValue/fields/waterTurbidity/doubleValue", 20);
    // content.set("fields/node1/mapValue/fields/waterTurbidityStatus/stringValue", "tidak keruh");
    // content.set("fields/node1/mapValue/fields/rainIntensity/doubleValue", 30);
    // content.set("fields/node1/mapValue/fields/rainIntensityStatus/stringValue", "tidak hujan");
    // updateMask += "node1.levelDanger, node1.waterLevel, node1.waterLevelStatus, node1.waterTurbidity, node1.waterTurbidityStatus, node1.rainIntensity, node1.rainIntensityStatus,";

    // // Node 2
    // content.set("fields/node2/mapValue/fields/levelDanger/stringValue", "aman");
    // content.set("fields/node2/mapValue/fields/waterLevel/doubleValue", 40);
    // content.set("fields/node2/mapValue/fields/waterLevelStatus/stringValue", "sedang");
    // content.set("fields/node2/mapValue/fields/waterTurbidity/doubleValue", 50);
    // content.set("fields/node2/mapValue/fields/waterTurbidityStatus/stringValue", "keruh");
    // content.set("fields/node2/mapValue/fields/rainIntensity/doubleValue", 60);
    // content.set("fields/node2/mapValue/fields/rainIntensityStatus/stringValue", "hujan");
    // updateMask += "node2.levelDanger, node2.waterLevel, node2.waterLevelStatus, node2.waterTurbidity, node2.waterTurbidityStatus, node2.rainIntensity, node2.rainIntensityStatus,";
    // String tanggal = "2024-02-01";
    // String jam = "01:00";
    // delay(2000);
    // if (connection->ready())
    // {
    //     connection->updateDataRealtimeFirebase(&content, updateMask.c_str());
    //     connection->updateDataHistoryFirebase(&content, tanggal.c_str(), jam.c_str());
    // }
}

void loop()
{
    String data = myLora->onReceive();
    LoRa.onReceive();
    if (data != "")
    {
        Serial.println(data);
    }
    // connection->getCurrentTime(waktu);
    // if (connection->ready())
    // {
    // connection->updateDataRealtimeFirebase(&content, updateMask.c_str());
    // }
    // String valStr(waktu->date);
    // Serial.print("Tanggal : ");
    // Serial.println(waktu->date);
    // Serial.print("Jam : ");
    // Serial.println(waktu->time);
    // Serial.print("onlyJam : ");
    // Serial.println(22 == waktu->getHour());
    // delay(2000);
}
