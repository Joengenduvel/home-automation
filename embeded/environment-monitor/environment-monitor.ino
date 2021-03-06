#include "arduino_secrets.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include "DHT.h"

#define DHTPIN 12
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

const int sleepTime = 10*60*1e6; //10 minutes
const int maxWorkTime = 10000; //10 seconds

const char* ssid = SECRET_SSID;
const char* password =  SECRET_PASSWD;

const char* mqttServer = "docker-master-1";
const int mqttPort = 1883;

WiFiClient espClient;
PubSubClient client(espClient);
ADC_MODE(ADC_VCC);

void setup() {
  WiFi.begin(ssid, password);

  dht.begin();
  String connectionId = String("environment-monitor: " + String(ESP.getChipId()));
  String tempTopic = String("temp/" + String(ESP.getChipId()));
  String humidTopic = String("humid/" + String(ESP.getChipId()));
  String voltTopic = String("battery/3.3/" + String(ESP.getChipId()));
  
  String temperature = String(dht.readTemperature());
  String humidity = String(dht.readHumidity());

  float vdd = 0;
  int measurements = 1;
  vdd += ESP.getVcc() / 1024.0;
  while (WiFi.status() != WL_CONNECTED && millis() < maxWorkTime) {
    vdd += ESP.getVcc() / 1024.0;
    measurements++;
    delay(500);
  }
  vdd = vdd/measurements;
  String voltage = String(vdd);

    
  client.setServer(mqttServer, mqttPort);
  while (!client.connected() && millis() < maxWorkTime) {
    Serial.print(".");
    if (!client.connect(connectionId.c_str())) {
      return;
    }
  }

  client.loop();
  client.publish(humidTopic.c_str(), humidity.c_str());
  delay(60);
  client.publish(tempTopic.c_str(), temperature.c_str());
  delay(60);
  client.publish(voltTopic.c_str(), voltage.c_str());
  client.loop();
  client.disconnect();
}

void loop() {
  ESP.deepSleep(sleepTime);
}
