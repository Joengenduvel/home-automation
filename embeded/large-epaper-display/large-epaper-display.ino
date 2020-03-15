#include <DHT.h>
#include <DHT_U.h>

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

GxEPD2_3C < GxEPD2_750c, GxEPD2_750c::HEIGHT / 2 > display(GxEPD2_750c(/*CS=15*/ SS, /*DC=4*/ 4, /*RST=5*/ 5, /*BUSY=16*/ 16));

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
float precipitations[] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
String times[] = {"03:00", "06:00", "09:00", "12:00", "15:00", "18:00", "21:00", "00:00", "03:00"};

const int sleepTime = 10 * 60 * 1000; //10 minutes
unsigned long messageReceivedWaitTime = 1000; //1 second
unsigned long lastUpdateMillis = messageReceivedWaitTime;
String updatedAt = times[0];

enum states {CONNECT, SUBSCRIBE, RECEIVE_DATA, UPDATE_DISPLAY, ERROR};
uint8_t state = ERROR;
String errorMessage = "";

const char* ssid = SECRET_SSID;
const char* password =  SECRET_PASSWD;

const char* mqttServer = "192.168.86.20";
const int mqttPort = 1883;
String connectionId = "";

WiFiClient espClient;
PubSubClient client(espClient);

//Constants
#define DHTPIN 0     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE); //// Initialize DHT sensor for normal 16mhz Arduino
float internalTemp = 0;


void setup() {
  Serial.begin(115200);
  connectionId = String("display: " + String(ESP.getChipId()));

  WiFi.mode( WIFI_STA );
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
  dht.begin();
  
  state = CONNECT;
}

void loop() {
  client.loop();
  switch (state) {
    case CONNECT:
      reconnect();
      state = SUBSCRIBE;
      break;
    case SUBSCRIBE:
      if (!client.connected()) {
        state = CONNECT;
      } else {
        subscribe();
        state = RECEIVE_DATA;
      }
      break;
    case RECEIVE_DATA:
      if (!client.connected()) {
        state = CONNECT;
      } else {
        if ((unsigned long)(millis() - lastUpdateMillis) >= messageReceivedWaitTime && updatedAt != times[0]) {

          Serial.println("data complete");


          internalTemp = dht.readTemperature();
          Serial.print("internal temp");
          Serial.println(internalTemp);

          client.disconnect();
          state = UPDATE_DISPLAY;

        } else {
          //still waiting
          delay(100);
        }
      }

      break;
    case UPDATE_DISPLAY:
      Serial.println("updating display");
      updatedAt = times[0];
      updateDisplay(displayWeatherCallback);
      state = RECEIVE_DATA;
      break;
    default:
      updateDisplay(displayErrorCallback);
      gotoSleep();
      break;
  }
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
    client.connect(connectionId.c_str());
  }
}

void subscribe() {
  Serial.println("subscribing");

  client.subscribe("weather/actual/temperature");
  client.subscribe("weather/actual/icon");
  client.subscribe("weather/actual/windSpeed");
  client.subscribe("weather/actual/windDirection");
  client.subscribe("weather/actual/precipitation");
  client.subscribe("weather/actual/time");

  client.subscribe("weather/prediction/+/icon");
  client.subscribe("weather/prediction/+/temperature");
  client.subscribe("weather/prediction/+/time");
  client.subscribe("weather/prediction/+/precipitation");

  Serial.println("subscribed");
}

void updateDisplay(void (*drawCallback)(const void*)) {
  Serial.println("updating display");
  display.init(115200);
  display.setRotation(0);
  display.setFullWindow();
  display.firstPage();
  display.drawPaged(drawCallback, 0);
  display.powerOff();
  Serial.println("powering display off");
}

void displayErrorCallback(const void*) {
  display.fillScreen(GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);
  display.setTextSize(4);
  display.setCursor(0, 0);
  display.println("ERROR");
}

void displayWeatherCallback(const void*)
{
  display.fillScreen(GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(times[0]);
  //internal
  display.setTextSize(1);
  display.setCursor(0, margin * 2 -10);
  display.print("in");
  printTemperature(margin, margin * 2, internalTemp, true);
  // external
  display.setTextSize(1);
  display.setCursor(0, margin * 2 + 110);
  display.print("out");
  printTemperature(margin, margin * 2 + 120, temperatures[0], true);
  drawCompass(margin + 250, margin, windSpeeds[0], windDirections[0]);
  drawWeatherIcon(440, 0, icons[0], true);
  if (precipitations[0] > 0) {
    printPercipitation(500, imageSizeXL, precipitations[0], true);
  }
  for (int i = 1; i < 9; i++) {
    int x = margin + (imageSize * (i - 1));
    int y = 280;
    printTemperature(x + margin * 2, y - 5, temperatures[i], false);
    drawWeatherIcon(x, y, icons[i], false);
    if (icons[i] & rain || icons[i] & snow) {
      printPercipitation(x + margin * 2, y + imageSize, precipitations[i], false);
    }
    printTime(x + margin, y + imageSize + margin, times[i], false);
  }
}

void drawCompass(uint16_t x, uint16_t y, int windSpeed, int windDirection) {

  switch ((((windDirection + 180) % 360) + 22) / 45) {
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
  display.print(windSpeed);
  display.setTextSize(2);
  display.print(" m/s");
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
  String payload = toString(payloadArray,  length);
  String topic = String(topicArray);

  Serial.print(topic);
  Serial.print("=");
  Serial.println(payload);

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
      precipitations[index] = payload.toFloat();
    }
  }
  lastUpdateMillis = millis();
}

void printTemperature(uint16_t x, uint16_t y, float temp, bool isXL) {
  display.setTextColor(GxEPD_BLACK);
  display.setTextSize(isXL ? 9 : 2);
  display.setCursor(x, y);
  display.println(temp, 1);
}

void printTime(uint16_t x, uint16_t y, String timestamp, bool isXL) {
  display.setTextColor(GxEPD_BLACK);
  display.setTextSize(isXL ? 9 : 2);
  display.setCursor(x, y);
  display.println(timestamp);
}

void printPercipitation(uint16_t x, uint16_t y, float percipitation, bool isXL) {
  display.setTextColor(GxEPD_BLACK);
  display.setTextSize(isXL ? 3 : 1);
  display.setCursor(x, y);
  display.print(percipitation, 2);
  display.print(" mm");
}

String toString(byte * payloadArray, unsigned int length) {
  char buff_p[length];
  for (int i = 0; i < length; i++)
  {
    buff_p[i] = (char)payloadArray[i];
  }
  buff_p[length] = '\0';
  return String(buff_p);
}
void catchError(String message) {
  state = ERROR;
  errorMessage = message;
}

void gotoSleep() {
  Serial.println("GOING TO SLEEP");
  WiFi.mode( WIFI_OFF );
  WiFi.forceSleepBegin();
  ESP.deepSleep(0);
}
