#define CONFIG_ARDUHAL_LOG_COLORS 1
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
// ===================== LoRa Library & Config =====================
#include <MyLora.h>

const long frequency = 433E6; // LoRa Frequency
const byte nssPin = 5;        // LoRa radio chip select
const byte resetPin = 14;     // LoRa radio reset
const byte dio0Pin = 2;       // change for your board; must be a hardware interrupt pin
String data;
byte localAddress = 0xFF; // address this device
MyLora *myLora = new MyLora(&nssPin, &resetPin, &dio0Pin, &localAddress);
// =================================================================
#include <Network.h>
#include <env.cpp>
uint8_t buzzerPin = 4;

Network *connection = new Network();
// Waktu *waktu = new Waktu();

#include <DataNode.cpp>
DataNode *node1 = new DataNode("node1");
DataNode *node2 = new DataNode("node2");

TaskHandle_t handleConnectWifi = NULL;
TaskHandle_t handleReceiveLora = NULL;
TaskHandle_t handleBuzzer = NULL;
TaskHandle_t handleUpdateRealtime = NULL;
TaskHandle_t handleHistoryAndNotification = NULL;
TaskHandle_t handleDetermineLevel = NULL;

SemaphoreHandle_t xHTTPaccess = NULL;
QueueHandle_t parseQueue;

/**
 * Alur Task
 * 1. Receive Lora✔️ -> Parse Data✔️ -> Determine Level Danger✔️ -> Buzzer✔️ -> (2)
 * 2. Connect Wifi & Firebase✔️ |-> send Realtime every 5s
 *                              |-> send notif and history if danger
 */

void receiveLoraTask(void *pvParameter);
void buzzerTask(void *pvParameter);
void connectWifi(void *pvParameter);
void determineLevelOfDanger(void *pvParameter);
void historyAndNotificationTask(void *pvParameter);
void updateDataRealtime(void *pvParameter);
void parsingDataTask(void *pvParemeter);

enum level
{
    AMAN,
    WASPADA,
    BAHAYA
};

void setup()
{
    Serial.begin(115200);
    Serial.println("STARTING !!!");

    xHTTPaccess = xSemaphoreCreateMutex();
    parseQueue = xQueueCreate(5, sizeof(String));
    level levelDanger;

    // core 0 for RF app like BLE, and Wifi: use it for task have little execution time
    xTaskCreatePinnedToCore(buzzerTask, "Buzzer Task", 2048, NULL, 1, &handleBuzzer, 0);                         // 2KB
    xTaskCreatePinnedToCore(parsingDataTask, "Parsing Data", 5120, NULL, 3, NULL, 0);                            // 2KB
    xTaskCreatePinnedToCore(determineLevelOfDanger, "Determine Level", 2048, NULL, 2, &handleDetermineLevel, 0); // 2KB

    // core 1 for other app
    xTaskCreatePinnedToCore(receiveLoraTask, "Receive Lora", 2048, NULL, 4, &handleReceiveLora, 1);                                  // 2KB
    xTaskCreatePinnedToCore(connectWifi, "Connect Wifi", 5120, NULL, 3, &handleConnectWifi, 1);                                      // 5KB
    xTaskCreatePinnedToCore(historyAndNotificationTask, "History & Notification", 10240, NULL, 2, &handleHistoryAndNotification, 1); // 10KB
    xTaskCreatePinnedToCore(updateDataRealtime, "Update Realtime", 10240, NULL, 1, &handleUpdateRealtime, 1);                        // 10KB
}

void loop() {}
// =================================================================================
void receiveLoraTask(void *pvParameter)
{
    Serial.println(F("Create Receive Data Lora Task"));
    myLora->initilize(frequency);
    String message = "";
    ESP_LOGI("RECEIVE LORA", "Start to receive Message");
    while (true)
    {
        message = myLora->onReceive();
        if (message != "")
        {
            // Serial.println(message);
            if (xQueueSend(parseQueue, &message, pdMS_TO_TICKS(100)) != pdPASS)
            {
                ESP_LOGE("RECEIVE LORA", "Gagal Mengirim ke Queue Parse");
            }
            else
            {
                ESP_LOGI("RECEIVE LORA", "Berhasil Mengirim ke Queue Parse");
            }
        }
        vTaskDelay(20 / portTICK_PERIOD_MS); // 20ms
    }
}
// =================================================================================
void setFloatData(FirebaseJson *json, FirebaseJsonData *data, float *var, const char *path)
{
    json->get(*data, path);
    if (!data->success)
    {
        ESP_LOGE("SET FLOAT", "Error get data %s", path);
        return;
    }
    if (data->type == "int")
    {
        *var = data->to<float>();
        return;
    }

    *var = data->floatValue;
}

void setStringData(FirebaseJson *json, FirebaseJsonData *data, String *var, const char *path)
{
    json->get(*data, path);
    if (!data->success)
    {
        ESP_LOGE("SET String", "Error get data %s", path);
        return;
    }
    *var = data->stringValue;
}

void parsingDataTask(void *pvParemeter)
{
    Serial.println(F("Create Parsing Data Task"));
    FirebaseJson *json = new FirebaseJson;
    FirebaseJsonData *data = new FirebaseJsonData;
    String message;
    while (true)
    {
        // ensures delay calls are not missed due to use of continue
        vTaskDelay(20 / portTICK_PERIOD_MS); // 1s

        if (xQueueReceive(parseQueue, &message, pdTICKS_TO_MS(100)) == pdPASS)
        {
            Serial.println(message);
            if (!json->setJsonData(message))
            {
                ESP_LOGE("PARSING DATA", "Error parsing string to json");
                continue;
            }
            // set Data Node 1
            setFloatData(json, data, &node1->waterValue, "node1/w");
            setStringData(json, data, &node1->waterStatus, "node1/ws");
            setFloatData(json, data, &node1->turbidityValue, "node1/t");
            setStringData(json, data, &node1->turbidityStatus, "node1/ts");
            setFloatData(json, data, &node1->rainValue, "node1/r");
            setStringData(json, data, &node1->rainStatus, "node1/rs");

            // set Data Node 2
            setFloatData(json, data, &node2->waterValue, "node2/w");
            setStringData(json, data, &node2->waterStatus, "node2/ws");
            setFloatData(json, data, &node2->turbidityValue, "node2/t");
            setStringData(json, data, &node2->turbidityStatus, "node2/ts");
            setFloatData(json, data, &node2->rainValue, "node2/r");
            setStringData(json, data, &node2->rainStatus, "node2/rs");
            ESP_LOGI("PARSING DATA", "Parsing Data Done");

            // Notify to Determine Level Task that data ready
            xTaskNotify(handleDetermineLevel, 1, eNoAction);
        }
    }
}
// =================================================================================
void determineLevelOfDanger(void *pvParameter)
{
    Serial.println(F("Create Determine Level Danger Task"));
    uint32_t ulNotificationValue;
    level levelDanger;
    while (true)
    {
        if (xTaskNotifyWait(0x00, 0x00, &ulNotificationValue, pdMS_TO_TICKS(100)) == pdPASS)
        {
            node1->determineLevel();
            node2->determineLevel();

            if (node1->levelDanger == "Waspada" || node2->levelDanger == "Waspada")
            {
                levelDanger = BAHAYA;
            }
            else if (node1->levelDanger == "Siaga" || node2->levelDanger == "Siaga")
            {
                levelDanger = WASPADA;
            }
            else
            {
                levelDanger = AMAN;
            }
            ESP_LOGI("DETEMINE LEVEL", "Determine Level Done with value %d", levelDanger);

            // Notify to Update Realtime Task Level Danger has Determine
            if (eTaskGetState(handleUpdateRealtime) != eSuspended)
            {
                xTaskNotify(handleUpdateRealtime, 1, eNoAction);
            }

            // Notify to Notification & History Task Level Danger has Determine
            if (eTaskGetState(handleHistoryAndNotification) != eSuspended && (levelDanger != AMAN))
            {
                xTaskNotify(handleHistoryAndNotification, 1, eNoAction);
            }

            // Notify to Buzzer Task Level Danger has Determine
            xTaskNotify(handleBuzzer, (uint32_t)levelDanger, eSetValueWithOverwrite);
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
// =================================================================================
void buzzerTask(void *pvParameter)
{
    Serial.println(F("Create Buzzer Task"));
    pinMode(buzzerPin, OUTPUT);
    bool buzzerState = 0;
    digitalWrite(buzzerPin, buzzerState);
    uint16_t buzzerOn = 1000;
    uint16_t interval;
    uint32_t levelDanger;
    uint64_t currentTime, lastToggle;

    while (true)
    {
        if (xTaskNotifyWait(0x00, ULONG_MAX, &levelDanger, pdMS_TO_TICKS(100)) == pdPASS)
        {
            switch ((level)levelDanger)
            {
            case BAHAYA:
                interval = 1000;
                buzzerState = 1;
                digitalWrite(buzzerPin, buzzerState);
                break;
            case WASPADA:
                interval = 5000;
                buzzerState = 1;
                digitalWrite(buzzerPin, buzzerState);
                break;
            default:
                interval = 0;
                break;
            }
        }

        if (interval > 0)
        {
            currentTime = millis();
            if (buzzerState)
            {
                if ((currentTime - lastToggle) > buzzerOn)
                {
                    // Serial.println("Change State to Off");
                    buzzerState = !buzzerState;
                    digitalWrite(buzzerPin, buzzerState);
                    lastToggle = currentTime;
                }
            }
            else
            {
                if ((currentTime - lastToggle) > interval)
                {
                    // Serial.println("Change State to On");
                    buzzerState = !buzzerState;
                    digitalWrite(buzzerPin, buzzerState);
                    lastToggle = currentTime;
                }
            }
        }
        else
        {
            digitalWrite(buzzerPin, LOW);
        }
        vTaskDelay(50 / portTICK_PERIOD_MS); // 50ms
    }
}
// =================================================================================
void suspendTask()
{
    vTaskSuspend(handleHistoryAndNotification);
    ESP_LOGI("CONNECT WIFI", "Suspend Notification & History Task");
    vTaskSuspend(handleUpdateRealtime);
    ESP_LOGI("CONNECT WIFI", "Suspend Update Realtime Task");
}

void resumeTask()
{
    vTaskResume(handleHistoryAndNotification);
    ESP_LOGI("CONNECT WIFI", "Resume Notification & History Task");
    vTaskResume(handleUpdateRealtime);
    ESP_LOGI("CONNECT WIFI", "Resume Update Realtime Task");
}
void connectWifi(void *pvParameter)
{
    Serial.println(F("Create Connect Wifi Task"));
    Serial.printf("Wifi SSID : %s\n", WIFI_SSID);
    while (true)
    {
        if (WiFi.status() != WL_CONNECTED)
        {
            if (eTaskGetState(handleUpdateRealtime) != eSuspended || eTaskGetState(handleHistoryAndNotification) != eSuspended)
            {
                suspendTask();
            }

            if (connection->initializeWifi(WIFI_SSID, WIFI_PASSWORD))
            {
                connection->initializeTime(NTP_GMT_OFFSET_SEC, NTP_DAYLIGHT_OFFSET_SEC, NTP_SERVER);
                connection->initializeFirebase(API_KEY, FIREBASE_PROJECT_ID, USER_EMAIL, USER_PASSWORD, ID_SUNGAI, CLIENT_EMAIL, PRIVATE_KEY);
                ESP_LOGI("CONNECT_WIFI", "Succcess connected to Wifi, Syncronize Time and Connected to Firebase");

                if (eTaskGetState(handleUpdateRealtime) == eSuspended || eTaskGetState(handleHistoryAndNotification) == eSuspended)
                {
                    if (connection->ready())
                    {
                        resumeTask();
                    }
                }
            }
        }
        else if (WiFi.status() == WL_CONNECTED)
        {
            if (eTaskGetState(handleUpdateRealtime) == eSuspended || eTaskGetState(handleHistoryAndNotification) == eSuspended)
            {
                if (connection->ready())
                {
                    resumeTask();
                }
            }
        }
        vTaskDelay(5000 / portTICK_PERIOD_MS); // 5s
    }
}
// =================================================================================
void updateDataRealtime(void *pvParameter)
{
    Serial.println(F("Create Update Data Task"));
    bool newData = false;
    uint16_t interval = 5000; // 5000ms or 5s
    uint32_t ulNotificationValue;
    uint64_t prevSend = 0;
    uint64_t current = millis();
    String updateMask;
    FirebaseJson content;
    while (true)
    {
        if (xTaskNotifyWait(0x00, 0x00, &ulNotificationValue, pdMS_TO_TICKS(100)) == pdPASS)
        {
            newData = true;
        }
        // if (connection->ready() && (millis() - prevSend) > interval)
        current = millis();
        if ((current - prevSend) > interval && newData)
        {
            if (connection->ready())
            {
                if (xSemaphoreTake(xHTTPaccess, portMAX_DELAY) == pdTRUE)
                {
                    updateMask = "";
                    // Node 1
                    node1->toJson(&content);
                    updateMask += "node1.levelDanger, node1.waterLevel, node1.waterLevelStatus, node1.waterTurbidity, node1.waterTurbidityStatus, node1.rainIntensity, node1.rainIntensityStatus,";

                    // Node 2
                    node2->toJson(&content);
                    updateMask += "node2.levelDanger, node2.waterLevel, node2.waterLevelStatus, node2.waterTurbidity, node2.waterTurbidityStatus, node2.rainIntensity, node2.rainIntensityStatus,";

                    if (connection->updateDataRealtimeFirebase(&content, updateMask.c_str()))
                    {
                        ESP_LOGI("UPDATE REALTIME", "Success Update Realtime");
                        newData = false;
                    }
                    else
                    {
                        ESP_LOGE("UPDATE REALTIME", "Failed Update, check connection or data");
                    }
                    xSemaphoreGive(xHTTPaccess);
                }
                prevSend = current;
            }
            else
            {
                ESP_LOGE("UPDATE REALTIME", "Connection Not Ready");
            }
        }
        vTaskDelay(100 / portTICK_PERIOD_MS); // 1s
    }
}
// =================================================================================

void historyAndNotificationTask(void *pvParameter)
{
    Serial.println(F("Create Send Notification Task"));
    uint32_t ulNotificationValue;
    String prevLevelNode1 = "aman";
    String prevLevelNode2 = "aman";
    String titleNotification = "";
    String bodyNotification = "";
    String channelIdNotification = "";
    while (true)
    {
        if (xTaskNotifyWait(0x00, 0x00, &ulNotificationValue, pdMS_TO_TICKS(100)) == pdPASS)
        {
            if (xSemaphoreTake(xHTTPaccess, portMAX_DELAY) == pdTRUE)
            {
                if (connection->ready())
                {
                    ESP_LOGI("NOTIFICATION & HISTORY", "Send Notification");
                    // Bahaya Node 1
                    if (node1->levelDanger == "Waspada" && prevLevelNode1 != "Waspada")
                    {
                        // connection->sendHistory();
                        connection->sendNotification("Waspada", "Node 1 Dalam Keadaan Waspada", "danger_notification");
                    }
                    else if (node1->levelDanger == "Siaga" && prevLevelNode1 != "Siaga")
                    {
                        // connection->sendHistory();
                        connection->sendNotification("Siaga", "Node 1 Dalam Keadaan Waspada", "alert_notification");
                    }

                    // Bahaya Node 2
                    if (node2->levelDanger == "Waspada" && prevLevelNode2 != "Waspada")
                    {
                        // connection->sendHistory();
                        connection->sendNotification("Waspada", "Node 2 Dalam Keadaan Waspada", "danger_notification");
                    }
                    else if (node2->levelDanger == "Siaga" && prevLevelNode2 != "Siaga")
                    {
                        // connection->sendHistory();
                        connection->sendNotification("Siaga", "Node 2 Dalam Keadaan Waspada", "alert_notification");
                    }
                }
                xSemaphoreGive(xHTTPaccess);
            }
        }
        vTaskDelay(100 / portTICK_PERIOD_MS); // 100ms
    }
}
// =================================================================================