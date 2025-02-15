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
#include <time.h>
uint8_t buzzerPin = 4;

Network *connection = new Network();
Waktu *waktu = new Waktu();

#include <DataNode.cpp>
DataNode *node1 = new DataNode("node1");
DataNode *node2 = new DataNode("node2");

TaskHandle_t handleConnectWifi = NULL;
TaskHandle_t handleReceiveLora = NULL;
TaskHandle_t handleBuzzer = NULL;
TaskHandle_t handleUpdateRealtime = NULL;
TaskHandle_t handleSendNotification = NULL;
TaskHandle_t handleUpdateHistory = NULL;
TaskHandle_t handleDetermineLevel = NULL;

SemaphoreHandle_t xHTTPaccess = NULL;
QueueHandle_t parseQueue;

/**
 * Alur Task
 * 1. Receive Lora✔️ -> Parse Data✔️ -> Determine Level Danger✔️ -> Buzzer✔️ -> (2)
 * 2. Connect Wifi & Firebase✔️ |-> send Realtime every 5s✔️
 *                              |-> send notif and history if danger
 */

void receiveLoraTask(void *pvParameter);
void buzzerTask(void *pvParameter);
void connectWifi(void *pvParameter);
void determineLevelOfDanger(void *pvParameter);
void sendNotificationTask(void *pvParameter);
void updateDataRealtime(void *pvParameter);
void parsingDataTask(void *pvParemeter);
void updateHistory(void *pvParameter);

enum level
{
    SAFE,
    ALERT,
    DANGER
};

void setup()
{
    Serial.begin(115200);
    Serial.println("STARTING !!!");

    xHTTPaccess = xSemaphoreCreateMutex();
    parseQueue = xQueueCreate(5, sizeof(String));
    level levelDanger;

    // core 0 for RF app like BLE, and Wifi: use it for task have little execution time
    xTaskCreatePinnedToCore(parsingDataTask, "Parsing Data", 10240, NULL, 3, NULL, 0);                           // 10KB
    xTaskCreatePinnedToCore(determineLevelOfDanger, "Determine Level", 2048, NULL, 2, &handleDetermineLevel, 0); // 2KB
    xTaskCreatePinnedToCore(buzzerTask, "Buzzer Task", 2048, NULL, 1, &handleBuzzer, 0);                         // 2KB

    // core 1 for other app
    xTaskCreatePinnedToCore(receiveLoraTask, "Receive Lora", 2048, NULL, 5, &handleReceiveLora, 1);                      // 2KB
    xTaskCreatePinnedToCore(connectWifi, "Connect Wifi", 10240, NULL, 4, &handleConnectWifi, 1);                         // 10KB
    xTaskCreatePinnedToCore(sendNotificationTask, "History & Notification", 10752, NULL, 5, &handleSendNotification, 1); // 15KB
    xTaskCreatePinnedToCore(updateDataRealtime, "Update Realtime", 10240, NULL, 1, &handleUpdateRealtime, 1);            // 10KB
    xTaskCreatePinnedToCore(updateHistory, "Update History", 10240, NULL, 2, &handleUpdateHistory, 1);
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
            // Serial.println();
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
void setFloatData(FirebaseJson *json, FirebaseJsonData *data, float *var, const char *path, bool &error)
{
    json->get(*data, path);
    if (!data->success)
    {
        ESP_LOGE("SET FLOAT", "Error get data %s", path);
        error = true;
        return;
    }

    *var = data->floatValue;
}

void setStringData(FirebaseJson *json, FirebaseJsonData *data, String *var, const char *path, bool &error)
{
    json->get(*data, path);
    if (!data->success)
    {
        error = true;
        ESP_LOGE("SET String", "Error get data %s", path);
        return;
    }
    *var = data->stringValue;
}

void parsingDataTask(void *pvParemeter)
{
    Serial.println(F("Create Parsing Data Task"));
    FirebaseJson *json = new FirebaseJson();
    FirebaseJsonData *data = new FirebaseJsonData();
    String message;
    bool errorWhenParse;
    while (true)
    {
        // ensures delay calls are not missed due to use of continue
        vTaskDelay(20 / portTICK_PERIOD_MS); // 1s

        if (xQueueReceive(parseQueue, &message, pdTICKS_TO_MS(100)) == pdPASS)
        {
            if (!json->setJsonData(message))
            {
                Serial.println(message);
                ESP_LOGE("PARSING DATA", "Error parsing string to json");
                continue;
            }
            errorWhenParse = false;
            // set Data Node 1
            setFloatData(json, data, &node1->waterValue, "node1/w", errorWhenParse);
            setStringData(json, data, &node1->waterStatus, "node1/ws", errorWhenParse);
            setFloatData(json, data, &node1->turbidityValue, "node1/t", errorWhenParse);
            setStringData(json, data, &node1->turbidityStatus, "node1/ts", errorWhenParse);
            setFloatData(json, data, &node1->rainValue, "node1/r", errorWhenParse);
            setStringData(json, data, &node1->rainStatus, "node1/rs", errorWhenParse);

            // set Data Node 2
            setFloatData(json, data, &node2->waterValue, "node2/w", errorWhenParse);
            setStringData(json, data, &node2->waterStatus, "node2/ws", errorWhenParse);
            setFloatData(json, data, &node2->turbidityValue, "node2/t", errorWhenParse);
            setStringData(json, data, &node2->turbidityStatus, "node2/ts", errorWhenParse);
            setFloatData(json, data, &node2->rainValue, "node2/r", errorWhenParse);
            setStringData(json, data, &node2->rainStatus, "node2/rs", errorWhenParse);

            if (!errorWhenParse)
            {
                ESP_LOGI("PARSING DATA", "Parsing Data Done");

                // Notify to Determine Level Task that data ready
                xTaskNotify(handleDetermineLevel, 1, eNoAction);
            }
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

            node1->printAllData();
            node2->printAllData();

            if (node1->levelDanger == "Danger" || node2->levelDanger == "Danger")
            {
                levelDanger = DANGER;
            }
            else if (node1->levelDanger == "Alert" || node2->levelDanger == "Alert")
            {
                levelDanger = ALERT;
            }
            else
            {
                levelDanger = SAFE;
            }
            // ESP_LOGI("DETEMINE LEVEL", "Determine Level Done with value %d", levelDanger);
            if (levelDanger == DANGER)
            {
                ESP_LOGW("DETEMINE LEVEL", "Determine Level Done : Level is Danger");
                ESP_LOGW("BUZZER", "Buzzer On Interval 1s");
            }
            else if (levelDanger == ALERT)
            {
                ESP_LOGW("DETEMINE LEVEL", "Determine Level Done : Level is Alert");
                ESP_LOGW("BUZZER", "Buzzer On Interval 5s");
            }
            else
            {
                ESP_LOGW("DETEMINE LEVEL", "Determine Level Done : Level is Safe");
                ESP_LOGW("BUZZER", "Buzzer Off");
            }

            // Notify to Notification & History Task Level Danger has Determine
            if (eTaskGetState(handleSendNotification) != eSuspended && (levelDanger != SAFE))
            {
                xTaskNotify(handleSendNotification, 1, eNoAction);
            }

            // Notify to Update Realtime Task Level Danger has Determine
            if (eTaskGetState(handleUpdateRealtime) != eSuspended)
            {
                xTaskNotify(handleUpdateRealtime, 1, eNoAction);
            }

            // // Notify to Buzzer Task Level Danger has Determine
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
    level prevLevel = SAFE;
    uint16_t buzzerOn = 1000;
    uint16_t interval;
    uint32_t levelDanger;
    uint64_t currentTime, lastToggle;

    while (true)
    {
        if (xTaskNotifyWait(0x00, ULONG_MAX, &levelDanger, pdMS_TO_TICKS(100)) == pdPASS)
        {
            if ((level)levelDanger != prevLevel)
            {
                prevLevel = (level)levelDanger;
                switch ((level)levelDanger)
                {
                case DANGER:
                    interval = 1000;
                    buzzerState = 1;
                    digitalWrite(buzzerPin, buzzerState);
                    break;
                case ALERT:
                    interval = 5000;
                    buzzerState = 1;
                    digitalWrite(buzzerPin, buzzerState);
                    break;
                case SAFE:
                    interval = 0;
                    buzzerState = 0;
                    digitalWrite(buzzerPin, buzzerState);
                    break;
                default:
                    break;
                }
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
    vTaskSuspend(handleSendNotification);
    ESP_LOGI("CONNECT WIFI", "Suspend Send Notification Task");
    vTaskSuspend(handleUpdateHistory);
    ESP_LOGI("CONNECT WIFI", "Suspend Update History Task");
    vTaskSuspend(handleUpdateRealtime);
    ESP_LOGI("CONNECT WIFI", "Suspend Update Realtime Task");
}

void resumeTask()
{
    vTaskResume(handleSendNotification);
    ESP_LOGI("CONNECT WIFI", "Resume Send Notification Task");
    vTaskResume(handleUpdateHistory);
    ESP_LOGI("CONNECT WIFI", "Resume Update History Task");
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
            if (eTaskGetState(handleUpdateRealtime) != eSuspended || eTaskGetState(handleSendNotification) != eSuspended || eTaskGetState(handleUpdateHistory) != eSuspended)
            {
                suspendTask();
            }

            if (connection->initializeWifi(WIFI_SSID, WIFI_PASSWORD))
            {
                connection->initializeFirebase(API_KEY, FIREBASE_PROJECT_ID, USER_EMAIL, USER_PASSWORD, ID_SUNGAI, CLIENT_EMAIL, PRIVATE_KEY);
                connection->initializeTime(NTP_GMT_OFFSET_SEC, NTP_DAYLIGHT_OFFSET_SEC, NTP_SERVER);
                ESP_LOGI("CONNECT_WIFI", "Succcess connected to Wifi, Syncronize Time and Connected to Firebase");
                connection->getCurrentTime(waktu);
                ESP_LOGI("CURRENT TIME", "DATETIME : %s", waktu->fullDateTime().c_str());

                if (eTaskGetState(handleUpdateRealtime) == eSuspended || eTaskGetState(handleSendNotification) == eSuspended || eTaskGetState(handleUpdateHistory) == eSuspended)
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
            if (eTaskGetState(handleUpdateRealtime) == eSuspended || eTaskGetState(handleSendNotification) == eSuspended || eTaskGetState(handleUpdateHistory) == eSuspended)
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
    FirebaseJson *content = new FirebaseJson();
    while (true)
    {
        if (xTaskNotifyWait(0x00, 0xFF, &ulNotificationValue, pdMS_TO_TICKS(100)) == pdPASS)
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
                    node1->toJson(content);
                    updateMask += "node1.levelDanger, node1.waterLevel, node1.waterLevelStatus, node1.waterTurbidity, node1.waterTurbidityStatus, node1.rainIntensity, node1.rainIntensityStatus,";

                    // Node 2
                    node2->toJson(content);
                    updateMask += "node2.levelDanger, node2.waterLevel, node2.waterLevelStatus, node2.waterTurbidity, node2.waterTurbidityStatus, node2.rainIntensity, node2.rainIntensityStatus,";

                    if (connection->updateDataRealtimeFirebase(content, updateMask.c_str()))
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

void sendNotificationTask(void *pvParameter)
{
    Serial.println(F("Create Send Notification Task"));
    uint32_t ulNotificationValue;
    String prevLevelNode1 = "Safe";
    String prevLevelNode2 = "Safe";
    String channelIdNotification = "";
    while (true)
    {
        if (xTaskNotifyWait(0x00, 0xFF, &ulNotificationValue, pdMS_TO_TICKS(100)) == pdPASS)
        {
            if (xSemaphoreTake(xHTTPaccess, portMAX_DELAY) == pdTRUE)
            {
                if (connection->ready())
                {
                    delay(100);
                    // Bahaya Node 1
                    if (node1->levelDanger == "Danger" && prevLevelNode1 != "Danger")
                    {
                        connection->sendNotification("Node 1 is Alert", node1->payloadNotification().c_str(), "danger_notification");
                    }
                    else if (node1->levelDanger == "Alert" && prevLevelNode1 != "Alert")
                    {
                        connection->sendNotification("Node 1 is Danger", node1->payloadNotification().c_str(), "alert_notification");
                    }
                    prevLevelNode1 = node1->levelDanger;

                    // Bahaya Node 2
                    if (node2->levelDanger == "Danger" && prevLevelNode2 != "Danger")
                    {
                        // connection->sendHistory();
                        connection->sendNotification("Node 2 is Alert", node2->payloadNotification().c_str(), "danger_notification");
                    }
                    else if (node2->levelDanger == "Alert" && prevLevelNode2 != "Alert")
                    {
                        // connection->sendHistory();
                        connection->sendNotification("Node 2 is Danger", node2->payloadNotification().c_str(), "alert_notification");
                    }
                    prevLevelNode2 = node2->levelDanger;
                    xTaskNotify(handleUpdateHistory, 1, eNoAction);
                }
                xSemaphoreGive(xHTTPaccess);
            }
        }
        vTaskDelay(100 / portTICK_PERIOD_MS); // 100ms
    }
}
// =================================================================================
void updateHistory(void *pvParameter)
{
    Serial.println(F("Create Update History Task"));
    uint32_t ulNotificationValue;
    String prevLevelNode1 = "Safe";
    String prevLevelNode2 = "Safe";
    FirebaseJson *jsonNode = new FirebaseJson();
    while (true)
    {
        if (xTaskNotifyWait(0x00, 0x00, &ulNotificationValue, pdMS_TO_TICKS(100)) == pdPASS)
        {
            connection->getCurrentTime(waktu);
            if (xSemaphoreTake(xHTTPaccess, portMAX_DELAY) == pdTRUE)
            {
                if (connection->ready())
                {
                    if (node1->levelDanger == "Danger" && prevLevelNode1 != "Danger")
                    {
                        node1->toJsonHistory(jsonNode, waktu->fullDateTime());
                        connection->updateDataHistoryFirebase(jsonNode);
                    }
                    else if (node1->levelDanger == "Alert" && prevLevelNode1 != "Alert")
                    {
                        node1->toJsonHistory(jsonNode, waktu->fullDateTime());
                        connection->updateDataHistoryFirebase(jsonNode);
                    }
                    prevLevelNode1 = node1->levelDanger;

                    if (node2->levelDanger == "Danger" && prevLevelNode2 != "Danger")
                    {
                        node2->toJsonHistory(jsonNode, waktu->fullDateTime());
                        connection->updateDataHistoryFirebase(jsonNode);
                    }
                    else if (node2->levelDanger == "Alert" && prevLevelNode2 != "Alert")
                    {
                        node2->toJsonHistory(jsonNode, waktu->fullDateTime());
                        connection->updateDataHistoryFirebase(jsonNode);
                    }
                    prevLevelNode2 = node2->levelDanger;
                }
                xSemaphoreGive(xHTTPaccess);
            }
        }
        // Serial.println(uxTaskGetStackHighWaterMark(NULL));
        vTaskDelay(100 / portTICK_PERIOD_MS); // 100ms
    }
}