#include <stdint.h>
#include <string.h>

#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#include <TM1637Display.h>

#define PIN_TICK    D4
#define PIN_CLK     D3
#define PIN_DIO     D2

#define MQTT_HOST   "mosquitto.space.revspace.nl"
#define MQTT_PORT   1883

#define TOPIC_GEIGER    "revspace/sensors/geiger"

#define LOG_PERIOD 10

static char esp_id[16];

static WiFiManager wifiManager;
static WiFiClient wifiClient;
static PubSubClient mqttClient(wifiClient);
static TM1637Display display(PIN_CLK, PIN_DIO);

// the total count value
static volatile unsigned long counts = 0;

static int secondcounts[60];
static unsigned long int secidx_prev = 0;
static unsigned long int count_prev = 0;
static unsigned long int second_prev = 0;

// interrupt routine
ICACHE_RAM_ATTR static void tube_impulse(void)
{
    counts++;
}

void setup(void)
{
    // initialize serial port
    Serial.begin(115200);
    Serial.println("GEIGER\n");

    // get ESP id
    sprintf(esp_id, "%08X", ESP.getChipId());
    Serial.print("ESP ID: ");
    Serial.println(esp_id);

    // connect to wifi
    Serial.println("Starting WIFI manager ...");
    wifiManager.setConfigPortalTimeout(120);
    wifiManager.autoConnect("ESP-GEIGER");

    // start counting
    memset(secondcounts, 0, sizeof(secondcounts));
    Serial.println("Starting count ...");
    
    pinMode(PIN_TICK, INPUT);
    attachInterrupt(digitalPinToInterrupt(PIN_TICK), tube_impulse, FALLING); 
}

static bool mqtt_send(const char *topic, const char *value, bool retained)
{
    bool result = false;
    if (!mqttClient.connected()) {
        Serial.print("Connecting MQTT...");
        mqttClient.setServer(MQTT_HOST, MQTT_PORT);
        result = mqttClient.connect(esp_id, topic, 0, retained, "offline");
        Serial.println(result ? "OK" : "FAIL");
    }
    if (mqttClient.connected()) {
        Serial.print("Publishing ");
        Serial.print(value);
        Serial.print(" to ");
        Serial.print(topic);
        Serial.print("...");
        result = mqttClient.publish(topic, value, retained);
        Serial.println(result ? "OK" : "FAIL");
    }
    return result;
}

void loop()
{
    // update the circular buffer every second
    unsigned long int second = millis() / 1000;
    unsigned long int secidx = second % 60;
    if (secidx != secidx_prev) {
        // new second, store the counts from the last second
        unsigned long int count = counts;
        secondcounts[secidx_prev] = count - count_prev;
        count_prev = count;
        secidx_prev = secidx;
    }

    // report every LOG_PERIOD
    if ((second - second_prev) > LOG_PERIOD) {
        second_prev = second;

        // calculate sum
        int cpm = 0;
        for (int i = 0; i < 60; i++) {
            cpm += secondcounts[i];
        }

        // send over MQTT
        char message[16];
        snprintf(message, sizeof(message), "%d cpm", cpm);
        if (!mqtt_send(TOPIC_GEIGER, message, true)) {
            Serial.println("Restarting ESP...");
            ESP.restart();
        }

        // show on display
        display.showNumberDec(cpm);
    }

    // keep MQTT alive
    mqttClient.loop();
}

