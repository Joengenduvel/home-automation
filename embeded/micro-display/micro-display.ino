#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <ESP8266WiFi.h>
#include <MQTT.h>

#include "DHT.h"

#define OLED_RESET 0
Adafruit_SSD1306 display(OLED_RESET);

#define DHTPIN 2
// Uncomment whatever type you're using!
#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
DHT dht(DHTPIN, DHTTYPE);

const char* ssid = "";
const char* password =  "";

const char* mqttServer = "docker-master-1";
const int mqttPort = 1883;

WiFiClient espClient;
MQTTClient client;

void setup() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  
  displayText("BOOT", "DHT");
  dht.begin();

  displayText("BOOT", "WIFI");
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);    
    displayText("BOOT", "WIFI ...");
  }

  displayText("BOOT", "MQTT");
  client.begin("docker-master-1", espClient);
  client.onMessage(messageReceived);

  displayText("BOOT", "MQTT ...");
  while (!client.connected()) {
 
    if (client.connect("ESP8266Client")) {
 
      displayText("BOOT", "MQTT V"); 
      client.subscribe("temp/out");
 
    } else { 
      displayText("BOOT", "MQTT X");
      delay(2000);
      displayText("MQTT", String(client.lastError())); 
      delay(2000);
    }
}

  // Clear the buffer.
  display.clearDisplay();

}

void loop() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  client.publish("temp/in", String(t));
  client.publish("humid/in", String(h));

  if (isnan(h) || isnan(t)) {
    displayText("error", "dht");
    return;
  }
  displayText(String(h), "humid");
  delay(3000);
  displayText(String(t), "in temp");
  delay(6000);
}

void displayText(String textline1, String textline2) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(35,10);
  display.println(textline1);

  
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(35,25);
  display.println(textline2);
  
  display.display();
  client.loop();
}

void messageReceived(String &topic, String &payload) {
  //Serial.println("incoming: " + topic + " - " + payload);
}
