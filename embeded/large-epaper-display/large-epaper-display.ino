#define ENABLE_GxEPD2_GFX 0

#include <GxEPD2_BW.h>
#include <GxEPD2_3C.h>
#include "sun.h"


GxEPD2_3C < GxEPD2_750c, GxEPD2_750c::HEIGHT / 4 > display(GxEPD2_750c(/*CS=15*/ SS, /*DC=4*/ 4, /*RST=5*/ 5, /*BUSY=16*/ 16));

#define imageSize 75
#define imageSizeXL 200
#define margin 10

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("setup");
  display.init(115200);
  printOutsideIconActual();
  display.powerOff();
  Serial.println("setup done");
}

void loop() {
  // put your main code here, to run repeatedly:

}

void helloWorld()
{
  Serial.println("helloWorld");
  display.setRotation(0);
  //display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);
  uint16_t x = (display.width() - 160) / 2;
  uint16_t y = display.height() / 2;
  display.setFullWindow();
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(x, y);
    display.println("Hello World!");
    Serial.println("page");
  }
  while (display.nextPage());
  Serial.println("helloWorld done");
}

void printOutsideTemperatureActual() {
  Serial.println("printOutsideTemperatureActual");
  display.setRotation(0);
  display.setTextColor(GxEPD_BLACK);
  display.setTextSize(9);
  uint16_t x = 10;
  uint16_t y = 10;
  display.setFullWindow();
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(x, y);
    display.println("-33.00");
    Serial.println("page");
  }
  while (display.nextPage());
  Serial.println("printOutsideTemperatureActual done");
}


void printOutsideIconActual() {
  Serial.println("printOutsideTemperatureActual");
  display.setRotation(0);
  display.setFullWindow();
  display.firstPage();
  do
  {
    uint16_t x = 10;
    uint16_t y = 10;
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    display.setTextSize(9);
    display.setCursor(x, y);
    display.println("-33.00");
    display.setCursor(x, y+90);
    display.println("-33.00");
    drawWeatherIcons(x+400, y-10, 1, true);
    display.drawBitmap(x, y+170,  sunBig, 75, 75, GxEPD_YELLOW);
    display.drawBitmap(x + 75, y+170,  sunBig, 75, 75, GxEPD_YELLOW);
    display.drawBitmap(x + 75 * 2, y+170,  sunBig, 75, 75, GxEPD_YELLOW);
    display.drawBitmap(x + 75 * 3, y+170,  sunBig, 75, 75, GxEPD_YELLOW);
    display.drawBitmap(x + 75 * 4, y+170,  sunBig, 75, 75, GxEPD_YELLOW);
    display.drawBitmap(x + 75 * 5, y+170,  sunBig, 75, 75, GxEPD_YELLOW);
    display.drawBitmap(x + 75 * 6, y+170,  sunBig, 75, 75, GxEPD_YELLOW);
    display.drawBitmap(x + 75 * 7, y+170,  sunBig, 75, 75, GxEPD_YELLOW);
    Serial.println("page");
  }
  while (display.nextPage());
  Serial.println("printOutsideTemperatureActual done");
}

void drawWeatherIcons(uint16_t x, uint16_t y, char conditions, bool isXL){
    display.drawBitmap(x, y,  isXL? sunBig_XL: sunBig, isXL? 200:75, isXL? 200:75, GxEPD_YELLOW);
}

