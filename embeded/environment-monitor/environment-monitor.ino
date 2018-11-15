#include <ESP8266WiFi.h>
#include <MQTT.h>

#include "DHT.h"

#define OLED_RESET 0

#define DHTPIN 5
// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
DHT dht(DHTPIN, DHTTYPE);

const int sleepTime = 2*60*1e6; //2 minutes
const int maxWorkTime = 10000; //10 seconds

const char* ssid = SECRET_SSID;
const char* password =  SECRET_PASSWD;

const char* mqttServer = "docker-master-1";
const int mqttPort = 1883;

WiFiClient espClient;
MQTTClient client;

void setup() {

  WiFi.begin(ssid, password);

  dht.begin();
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (isnan(h) || isnan(t)) {
    return;
  }

  const char* connectionId = String("environmet-monitor: " + String(ESP.getChipId())).c_str();
  const String tempTopic = "temp/" + String(ESP.getChipId());
  const String humidTopic = "humid/" + String(ESP.getChipId());
  
  while (WiFi.status() != WL_CONNECTED && millis() < maxWorkTime) {
    delay(500);
  }

  client.begin("docker-master-1", espClient);

  while (!client.connected() && millis() < maxWorkTime) {

    if (!client.connect(connectionId)) {
      return;
    }
  }
  client.publish(tempTopic, String(t));
  client.publish(humidTopic, String(h));
  
}

void loop() {
  ESP.deepSleep(sleepTime);
}
