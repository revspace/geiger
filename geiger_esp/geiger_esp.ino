#include <stdint.h>

#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <PubSubClient.h>

#define MQTT_HOST   "mosquitto.space.revspace.nl"
#define MQTT_PORT   1883

#define TOPIC_GEIGER    "revspace/sensors/geiger"

#define LOG_PERIOD 15000        //Logging period in milliseconds, recommended value 15000-60000.
#define MAX_PERIOD 60000        //Maximum logging period without modifying this sketch

static char esp_id[16];

static WiFiManager wifiManager;
static WiFiClient wifiClient;
static PubSubClient mqttClient(wifiClient);

static unsigned long counts;
static unsigned long cpm;
static unsigned int multiplier;
static unsigned long previousMillis;

//subprocedure for capturing events from Geiger Kit
static void tube_impulse()
{
    counts++;
}

void setup()
{
    counts = 0;
    cpm = 0;
    multiplier = MAX_PERIOD / LOG_PERIOD;       //calculating multiplier, depend on your log period
    attachInterrupt(0, tube_impulse, FALLING);  //define external interrupts 

    // initialize serial port
    Serial.begin(115200);
    Serial.println("GEIGER\n");

    // get ESP id
    sprintf(esp_id, "%08X", ESP.getChipId());
    Serial.print("ESP ID: ");
    Serial.println(esp_id);

    Serial.println("Starting WIFI manager ...");
    wifiManager.autoConnect("ESP-ZG01");
}

static void mqtt_send(const char *topic, const char *value)
{
    if (!mqttClient.connected()) {
        mqttClient.setServer(MQTT_HOST, MQTT_PORT);
        mqttClient.connect(esp_id);
    }
    if (mqttClient.connected()) {
        Serial.print("Publishing ");
        Serial.print(value);
        Serial.print(" to ");
        Serial.print(topic);
        Serial.print("...");
        int result = mqttClient.publish(topic, value, true);
        Serial.println(result ? "OK" : "FAIL");
    }
}

void loop()
{
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis > LOG_PERIOD) {
        previousMillis = currentMillis;
        cpm = counts * multiplier;

        Serial.println(cpm);
        counts = 0;

        // send over MQTT
        char message[16];
        snprintf(message, sizeof(message), "%d cpm", cpm);
        mqtt_send(TOPIC_GEIGER, message);
    }
}

