#define CONFIG_ARDUHAL_LOG_COLORS 1
#include <Arduino.h>
#include <Firebase.h>

class DataNode
{
private:
    float prevWaterValue = 0.0F;
    bool checkKenaikanAir();
    String name;

public:
    float waterValue = 0.0F;
    String waterStatus = "aman";
    float turbidityValue = 0.0F;
    String turbidityStatus = "jernih";
    float rainValue = 0.0F;
    String rainStatus = "tidak hujan";
    String levelDanger = "aman";

    DataNode(const char *name);
    void explainWaterStatus();
    void explainTurbidityStatus();
    void explainRainStatus();
    void printAllData();
    void determineLevel();
    void toJson(FirebaseJson *json);
    void toJsonHistory(FirebaseJson *json, String datetime);
    String payloadNotification();
};

DataNode::DataNode(const char *nodeName)
{
    name = nodeName;
}

void DataNode::explainWaterStatus()
{
    if (waterStatus == "H")
    {
        waterStatus = "High";
    }
    else if (waterStatus == "M")
    {
        waterStatus = "Medium";
    }
    else if (waterStatus == "L")
    {
        waterStatus = "Low";
    }
    else
    {
        waterStatus = "Undefined";
    }
}
void DataNode::explainTurbidityStatus()
{
    if (turbidityStatus == "T")
    {
        turbidityStatus = "Turbid";
    }
    else if (turbidityStatus == "M")
    {
        turbidityStatus = "Murky";
    }
    else if (turbidityStatus == "CL")
    {
        turbidityStatus = "Clear";
    }
    else
    {
        turbidityStatus = "Undefined";
    }
}
void DataNode::explainRainStatus()
{
    if (rainStatus == "H")
    {
        rainStatus = "Heavy";
    }
    else if (rainStatus == "M")
    {
        rainStatus = "Moderate";
    }
    else if (rainStatus == "L")
    {
        rainStatus = "Light";
    }
    else
    {
        rainStatus = "No Rain";
    }
}

void DataNode::determineLevel()
{
    /**
     * Waspada if air tinggi atau hujan lebat dan air naik
     * Siaga if air sedang atau hujan lebat atau hujan lebat atau hujan sedang + air naik
     * Tinggi Air : L (Low), M (Medium), H (High)
     * Keruh Air : CL (Clear),M (Murky),T (Turbid)
     * Hujan : NR (No Rain),L (Light), M (Moderate), H (Heavy)
     * level : S (Safe), A (Alert), D (Danger)
     */
    explainWaterStatus();
    explainTurbidityStatus();
    explainRainStatus();

    if (waterStatus == "High")
    {
        levelDanger = "Danger";
        return;
    }

    if (rainStatus == "Heavy" && checkKenaikanAir())
    {
        levelDanger = "Danger";
        return;
    }

    if (waterStatus == "Medium" || turbidityStatus == "Turbid" || rainStatus == "Heavy")
    {
        levelDanger = "Alert";
        return;
    }

    if (rainStatus == "Medium" && checkKenaikanAir())
    {
        levelDanger = "Alert";
        return;
    }

    levelDanger = "Safe";
    return;
}

void DataNode::toJson(FirebaseJson *json)
{
    json->set("fields/" + name + "/mapValue/fields/levelDanger/stringValue", levelDanger);
    json->set("fields/" + name + "/mapValue/fields/waterLevel/doubleValue", waterValue);
    json->set("fields/" + name + "/mapValue/fields/waterLevelStatus/stringValue", waterStatus);
    json->set("fields/" + name + "/mapValue/fields/waterTurbidity/doubleValue", turbidityValue);
    json->set("fields/" + name + "/mapValue/fields/waterTurbidityStatus/stringValue", turbidityStatus);
    json->set("fields/" + name + "/mapValue/fields/rainIntensity/doubleValue", rainValue);
    json->set("fields/" + name + "/mapValue/fields/rainIntensityStatus/stringValue", rainStatus);
}

void DataNode::toJsonHistory(FirebaseJson *json, String datetime)
{
    json->set("fields/node/stringValue", name);
    json->set("fields/datetime/stringValue", datetime);
    json->set("fields/levelDanger/stringValue", levelDanger);
    json->set("fields/waterLevel/doubleValue", waterValue);
    json->set("fields/waterLevelStatus/stringValue", waterStatus);
    json->set("fields/waterTurbidity/doubleValue", turbidityValue);
    json->set("fields/waterTurbidityStatus/stringValue", turbidityStatus);
    json->set("fields/rainIntensity/doubleValue", rainValue);
    json->set("fields/rainIntensityStatus/stringValue", rainStatus);
}

String DataNode::payloadNotification()
{
    String payload = "";
    payload = "Water Level : " + String(waterValue) + " CM [" + waterStatus + "]\n";
    payload += "Turbidity : " + String(turbidityValue) + " ADC [" + turbidityStatus + "]\n";
    payload += "Rain Intensity : " + String(rainValue) + " mm/hr [" + rainStatus + "]";
    return payload;
}

bool DataNode::checkKenaikanAir()
{
    static uint64_t prev = 0;
    if (millis() - prev > 120000 || prev == 0) // every 2m
    {
        prevWaterValue = waterValue;
        prev = millis();
    }
    if (waterValue - prevWaterValue > 1)
    {
        // Serial.printf("Hujan pada %s menyebakan air naik", name);
        ESP_LOGW("KENAIKAN AIR", "Hujan pada %s menyebakan air naik", name);
        // prevWaterValue = waterValue;
        return true;
    }
    return false;
}

void DataNode::printAllData()
{
    Serial.printf("== Data %s ==\n", name);
    Serial.printf("levelDanger : %s\n", levelDanger);
    Serial.printf("waterLevel : %.2f\n", waterValue);
    Serial.printf("waterLevelStatus : %s\n", waterStatus);
    Serial.printf("waterTurbidity : %.2f\n", turbidityValue);
    Serial.printf("waterTurbidityStatus : %s\n", turbidityStatus);
    Serial.printf("rainIntensity : %.2f\n", rainValue);
    Serial.printf("rainIntensityStatus : %s\n", rainStatus);
}