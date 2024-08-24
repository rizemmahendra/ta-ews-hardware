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
    void determineLevel();
    void toJson(FirebaseJson *json);
    void toJsonHistory(FirebaseJson *json, String datetime);
    String payloadNotification();
};

DataNode::DataNode(const char *nodeName)
{
    name = nodeName;
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
    if (waterStatus == "H")
    {
        levelDanger = "Waspada";
        return;
    }

    if (rainStatus == "H" && checkKenaikanAir())
    {
        levelDanger = "Waspada";
        return;
    }

    if (waterStatus == "M" || turbidityStatus == "T" || rainStatus == "H")
    {
        levelDanger = "Siaga";
        return;
    }

    if (rainStatus == "M" && checkKenaikanAir())
    {
        levelDanger = "Siaga";
    }

    levelDanger = "Aman";
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
    payload += "Turbidity : " + String(turbidityValue) + " NTU [" + turbidityStatus + "]\n";
    payload += "Rain Intensity : " + String(rainValue) + " mm/hr [" + rainStatus + "]";
    return payload;
}

bool DataNode::checkKenaikanAir()
{
    static uint64_t prev = 0;
    if (millis() - prev > 120000) // every 2m
    {
        prevWaterValue = waterValue;
        prev = millis();
    }
    if (waterValue - prevWaterValue > 1)
    {
        return true;
    }
    return false;
}