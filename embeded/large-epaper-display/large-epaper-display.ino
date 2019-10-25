#define ENABLE_GxEPD2_GFX 0

#include "arduino_secrets.h"
#include <GxEPD2_3C.h>

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

//images generated with: http://javl.github.io/image2cpp/
//images based on https://openweathermap.org/themes/openweathermap/assets/vendor/owm/img/widgets/09d.png
#include "sun.h"
#include "moon.h"
#include "clouds.h"
#include "rain.h"
#include "lightning.h"
#include "snow.h"
#include "fog.h"
#include "compass.h"


GxEPD2_3C < GxEPD2_750c, GxEPD2_750c::HEIGHT / 4 > display(GxEPD2_750c(/*CS=15*/ SS, /*DC=4*/ 4, /*RST=5*/ 5, /*BUSY=16*/ 16));

#define imageSize 75
#define imageSizeXL 200
#define margin 10


const unsigned char sun = 0x1;
const unsigned char cloud = 0x2;
const unsigned char darkCloud = 0x4;
const unsigned char rain = 0x8;
const unsigned char lightning = 0x10;
const unsigned char snow = 0x20;
const unsigned char fog = 0x40;
const unsigned char moon = 0x80;

String openWeatherIds[] = {"01d", "02d", "03d", "04d", "09d", "10d", "11d", "13d", "50d"};
char translatedOpenWeatherIdsDay[] = {sun, sun | cloud, sun | cloud, sun | cloud | darkCloud, 0, 0, 0, 0, sun | cloud | darkCloud | rain, sun | cloud | rain, sun | cloud | darkCloud | lightning, 0, sun | cloud | darkCloud | snow};
char translatedOpenWeatherIdsNight[] = {moon, moon | cloud, moon | cloud, moon | cloud | darkCloud, 0, 0, 0, 0, moon | cloud | darkCloud | rain, moon | cloud | rain, moon | cloud | darkCloud | lightning, 0, moon | cloud | darkCloud | snow};

char icons[] = {fog, fog, fog, fog, fog, fog, fog, fog, fog};
float temperatures[] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
int windSpeeds[] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
int windDirections[] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
int precipitations[] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
String times[] = {"03:00", "06:00", "09:00", "12:00", "15:00", "18:00", "21:00", "00:00", "03:00"};

const int sleepTime = 10 * 60 * 1000; //10 minutes
unsigned long messageReceivedWaitTime = 60*1000; //1 minute
unsigned long lastUpdateMillis = messageReceivedWaitTime;
unsigned long lastScreenUpdate = 0;
bool receivedData = false;

const char* ssid = SECRET_SSID;
const char* password =  SECRET_PASSWD;

const char* mqttServer = "docker-master-1";
const int mqttPort = 1883;
String connectionId = "";

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  connectionId = String("display: " + String(ESP.getChipId()));

  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
  lastScreenUpdate = millis() - (sleepTime * 2); //first time, draw imediate
}

void reconnect() {
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.println(WiFi.status());
    }
    Serial.println("Connected");
  }

  while (!client.connected()) {
    Serial.print(".");

    if (client.connect(connectionId.c_str())) {
      Serial.println("subscribing");

      client.subscribe("weather/actual/temperature");
      client.subscribe("weather/actual/icon");
      //client.subscribe("weather/actual/windSpeed");
      client.subscribe("weather/actual/windDirection");
      client.subscribe("weather/actual/precipitation");

      client.subscribe("weather/prediction/+/icon");
      client.subscribe("weather/prediction/+/temperature");
      client.subscribe("weather/prediction/+/time");
      //client.subscribe("weather/prediction/+/precipitation");
      
      Serial.println("subscribed");
      client.loop();

    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  //waiting for all data to arrive.
  if ((unsigned long)(millis() - lastUpdateMillis) >= messageReceivedWaitTime && receivedData) {
    receivedData = false;
    Serial.println("Data complete");
    //only update the screen from time to time
    if ((unsigned long)(millis() - lastScreenUpdate) >= sleepTime) {
      lastScreenUpdate = millis();
      Serial.println("updating display");
      display.init(115200);
      display.setRotation(0);
      display.setFullWindow();
      display.firstPage();
      display.drawPaged(updateScreenCallback, 0);

      Serial.println(client.state());
      Serial.println("powering off");
      display.powerOff();
      delay(500);
    }
  }
  client.loop();
}

void updateScreenCallback(const void*)
{
  display.fillScreen(GxEPD_WHITE);
  // internal
  //printTemperature(margin, margin*2, -22, true);
  // external
  printTemperature(margin, margin * 2 + 100, temperatures[0], true);
  drawCompass(230, margin, windSpeeds[0], windDirections[0]);
  drawWeatherIcon(440, 0, icons[0], true);
  if (precipitations[0] > 0) {
    display.setTextColor(GxEPD_BLACK);
    display.setTextSize(3);
    display.setCursor(500, imageSizeXL);
    //display.print(precipitations[0]);
    //display.print("mm");
  }
  for (int i = 1; i < 9; i++) {
    int x = margin + (imageSize * (i - 1));
    int y = 280;
    printTemperature(x + margin * 2, y - 5, temperatures[i], false);
    drawWeatherIcon(x, y, icons[i], false);
    printTime(x + margin, y + imageSize + margin, times[i], false);
  }
  client.loop();
}

void drawCompass(uint16_t x, uint16_t y, int windSpeed, int windDirection) {
  
    Serial.print("dir: ");
    Serial.println((((windDirection+180)%360) + 22) / 45);
    Serial.print("speed: ");
    Serial.println(windSpeed);
    
  switch ((((windDirection+180)%360) + 22) / 45) {
    case 7:
      display.drawBitmap(x, y,  compassImage7, 100, 100 , GxEPD_BLACK);
      break;
    case 6:
      display.drawBitmap(x, y,  compassImage6, 100, 100 , GxEPD_BLACK);
      break;
    case 5:
      display.drawBitmap(x, y,  compassImage5, 100, 100 , GxEPD_BLACK);
      break;
    case 4:
      display.drawBitmap(x, y,  compassImage4, 100, 100 , GxEPD_BLACK);
      break;
    case 3:
      display.drawBitmap(x, y,  compassImage3, 100, 100 , GxEPD_BLACK);
      break;
    case 2:
      display.drawBitmap(x, y,  compassImage2, 100, 100 , GxEPD_BLACK);
      break;
    case 1:
      display.drawBitmap(x, y,  compassImage1, 100, 100 , GxEPD_BLACK);
      break;
    case 0:
    default:
      display.drawBitmap(x, y,  compassImage0, 100, 100 , GxEPD_BLACK);
  }
  display.setTextColor(GxEPD_BLACK);
  display.setTextSize(3);
  display.setCursor(x + 30, y + 110);
  //display.println(windSpeed);
}

void rotateBitmap(const uint8_t * bitmap, uint8_t width, uint8_t height, float angle) {

}

void drawWeatherIcon(uint16_t x, uint16_t y, char conditions, bool isXL) {
  if (conditions & sun) {
    display.drawBitmap(x, y,  isXL ? sunImage_XL : sunImage, isXL ? imageSizeXL : imageSize, isXL ? imageSizeXL : imageSize, GxEPD_YELLOW);
  }
  if (conditions & moon) {
    display.drawBitmap(x, y,  isXL ? moonImage_XL : moonImage, isXL ? imageSizeXL : imageSize, isXL ? imageSizeXL : imageSize, GxEPD_BLACK);
  }
  if (conditions & darkCloud) {
    display.drawBitmap(x, y,  isXL ? darkCloudImage_XL : darkCloudImage, isXL ? imageSizeXL : imageSize, isXL ? imageSizeXL : imageSize, GxEPD_BLACK);
  }
  if (conditions & cloud) {
    display.drawBitmap(x, y,  isXL ? cloudImage_XL : cloudImage, isXL ? imageSizeXL : imageSize, isXL ? imageSizeXL : imageSize, GxEPD_BLACK);
  }
  if (conditions & rain) {
    display.drawBitmap(x, y,  isXL ? rainImage_XL : rainImage, isXL ? imageSizeXL : imageSize, isXL ? imageSizeXL : imageSize, GxEPD_BLACK);
  }
  if (conditions & snow) {
    display.drawBitmap(x, y,  isXL ? snowImage_XL : snowImage, isXL ? imageSizeXL : imageSize, isXL ? imageSizeXL : imageSize, GxEPD_BLACK);
  }
  if (conditions & lightning) {
    display.drawBitmap(x, y,  isXL ? lightningImage_XL : lightningImage, isXL ? imageSizeXL : imageSize, isXL ? imageSizeXL : imageSize, GxEPD_YELLOW);
  }
  if (conditions & fog) {
    display.drawBitmap(x, y,  isXL ? fogImage_XL : fogImage, isXL ? imageSizeXL : imageSize, isXL ? imageSizeXL : imageSize, GxEPD_BLACK);
  }
}


char translateOpenWeatherId(String id) {
  if (id.startsWith("5")) {
    //fog, so no other calculations needed
    return fog;
  } else {
    char imagesToDraw = 0;
    int numberPart = id.substring(0, 2).toInt() - 1;
    if (id.charAt(2) == 'd') {
      return translatedOpenWeatherIdsDay[numberPart];
    } else {
      return translatedOpenWeatherIdsNight[numberPart];
    }
  }
}

void callback(char* topicArray, byte * payloadArray, unsigned int length) {
  byte* p = (byte*)malloc(length);
  memcpy(p, payloadArray, length);
  String payload = String((char*)p);
  free(p);
  payloadArray = {};
  String topic = String(topicArray);

  int index = 0;
  if (topic.startsWith("weather/prediction/")) {
    //calculate the index of the icon
    index = topic.substring(19, topic.indexOf('/', 19)).toInt() / 3;

  }
  if (index < 9) {

    Serial.print(topic);
    Serial.print("=");
    Serial.println(payload);

    if (topic.endsWith("icon")) {
      icons[index] = translateOpenWeatherId(payload);
    }
    if (topic.endsWith("temperature")) {
      temperatures[index] = payload.toFloat();
    }
    if (topic.endsWith("time")) {
      times[index] = payload.substring(0, 5);
    }
    if (topic.endsWith("Direction")) {
      windDirections[index] = payload.toInt();
    }
    if (topic.endsWith("Speed")) {
      windSpeeds[index] = payload.toFloat();
    }
    if (topic.endsWith("precipitation")) {
      precipitations[index] = payload.toInt();
    }
    receivedData = true;
  }

  lastUpdateMillis = millis();
}

void printTemperature(uint16_t x, uint16_t y, float temp, bool isXL) {
  display.setTextColor(GxEPD_BLACK);
  display.setTextSize(isXL ? 9 : 2);
  display.setCursor(x, y);
  display.println(round(temp));
}

void printTime(uint16_t x, uint16_t y, String timestamp, bool isXL) {
  display.setTextColor(GxEPD_BLACK);
  display.setTextSize(isXL ? 9 : 2);
  display.setCursor(x, y);
  display.println(timestamp);
}

