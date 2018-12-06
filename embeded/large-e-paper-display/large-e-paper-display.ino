#define ENABLE_GxEPD2_GFX 0
#include <GxEPD2_3C.h>
#include "rain.h"


const unsigned char sun = 0x1;
const unsigned char cloud = 0x2;
const unsigned char darkCloud = 0x4;
const unsigned char rain = 0x8;
const unsigned char lightning = 0x10;
const unsigned char snow = 0x20;
const unsigned char fog = 0x40;
const unsigned char moon = 0x80;

char weatherConditions[] = {};
String openWeatherIds[] = {"01d","02d","03d","04d","09d","10d","11d","13d","50d"};
char translatedOpenWeatherIdsDay[] = {sun, sun|cloud, cloud, cloud|darkCloud, 0, 0, 0, 0, cloud|darkCloud|rain, sun|cloud|rain, cloud|darkCloud|lightning, 0, cloud|darkCloud|snow};

GxEPD2_3C<GxEPD2_750c, GxEPD2_750c::HEIGHT / 4> display(GxEPD2_750c(/*CS=15*/ SS, /*DC=4*/ 4, /*RST=5*/ 5, /*BUSY=16*/ 16));

void setup() {
  Serial.begin(115200);
  display.init(115200);

  display.fillScreen(GxEPD_WHITE);
  display.drawBitmap(0, 0, rain_image, 48, 48, GxEPD_BLACK);
  display.powerOff();
}

void loop() {
  Serial.println("loop");
  for(int i=0; i<9; i++){
    Serial.println(openWeatherIds[i]);
    char imagesToDraw = translateOpenWeatherId(openWeatherIds[i]);
    drawWeatherSymbol(i*50, 0,imagesToDraw);
    delay(500);
    Serial.println(i);
  }
  Serial.println("end loop");
  delay(1000);
}

char translateOpenWeatherId(String id){
  if(id.startsWith("5")){
    //fog, so no other calculations needed
    Serial.println("50 - fog");
    return fog;
  }else{
    char imagesToDraw = 0;
    int numberPart=id.substring(0,2).toInt()-1;
    Serial.println(numberPart);
    return translatedOpenWeatherIdsDay[numberPart];
  }
}

void drawWeatherSymbol(int x, int y, char imagesToDraw){
  Serial.println("==============");
  if(imagesToDraw & sun){
    Serial.println("sun");
  }
  if(imagesToDraw & moon){
    Serial.println("moon");
  }
  if(imagesToDraw & darkCloud){
    Serial.println("darkCloud");
  }
  if(imagesToDraw & cloud){
    Serial.println("cloud");
  }
  if(imagesToDraw & rain){
    Serial.println("rain");
  }
  if(imagesToDraw & snow){
    Serial.println("snow");
  }
  if(imagesToDraw & lightning){
    Serial.println("lightning");
  }
  if(imagesToDraw & fog){
    Serial.println("fog");
  }
  Serial.println("==============");
}
