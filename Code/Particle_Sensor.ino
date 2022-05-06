/* Sensor Test sketch */
//Image Converter Used: http://www.rinkydinkelectronics.com/_t_doimageconverter565.php

#include "Adafruit_PM25AQI.h"
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include <TFT_eSPI.h> 
#include <Wire.h>
#include <SPI.h>
#include <NTPClient.h>           //https://github.com/taranais/NTPClient
#include <WiFi.h>
//#include "WiFi.h"
#include <WiFiUdp.h>
#include <HTTPClient.h>          //Needed for the date
#include "font.h"
#include "overlay.h"
#include "Wifi_Icons.h"

#define RXD2 16
#define TXD2 17
#define SEALEVELPRESSURE_HPA (1013.25)
#define temperaure_offset (6.7)  

Adafruit_BME680 bme;
Adafruit_PM25AQI aqi = Adafruit_PM25AQI();

TFT_eSPI tft = TFT_eSPI(); 
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
String formattedDate;
String dayStamp;
uint32_t targetTime = 0;      
const long interval = 60000;  //Update every minute
unsigned long previousMillis = 0;   

const char *ssid     = "SSID";        
const char *password = "PASSWORD";      


void setup() 
{
  tft.init();
  tft.setSwapBytes(true);
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);
  Serial.begin(115200);
  while (!Serial) delay(10);
  tft.setTextColor(TFT_GREEN,TFT_BLACK);
  tft.setTextSize(2);
  Serial.println("Wii Test");
  Serial.print("Connecting to ");
  tft.println("Connecting to Wifi");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) 
  {
   delay(500);
   Serial.print(".");
   tft.print(".");
  }
 
  tft.println(" ");
  tft.println(" ");
  tft.println("WiFi connected.");
  tft.println("IP address: ");
  tft.println(WiFi.localIP());
  Serial.println(WiFi.localIP());
  delay(3000);

  if (!bme.begin()) 
  {
   Serial.println("Could not find a valid BME680 sensor, check wiring!");
   while (1);
  }

  //Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); 

  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  
  if (! aqi.begin_UART(&Serial2)) 
  { 
   Serial.println("Could not find PM 2.5 sensor!");
   while (1) delay(10);
  }

  Serial.println("PM25 found!");
 
  tft.fillScreen(TFT_BLACK);
 
  tft.pushImage(0,0,240,240,overlay);
  
  timeClient.begin();
  timeClient.setTimeOffset(-10800);  //-14400 or -10800
  delay(500);
  timeClient.update();
}



void loop() 
{
 unsigned long currentMillis = millis(); 
 int currentHour;
 int currentMinute;
 int currentDay;
  
 tft.setTextColor(TFT_GREEN,TFT_BLACK);
 tft.setTextSize(2); 
 
 if (currentMillis - previousMillis >= interval) 
 {
  previousMillis = currentMillis; 
  timeClient.update();
  Serial.println("Time Updated");
 }

  tft.setTextSize(1);
  tft.setCursor (0, 7); 
  tft.print(__DATE__);
  tft.setCursor (100, 7); 
  tft.print(__TIME__);
  
  tft.setTextSize(2);  //fixes font for PM2.5
  WifiIcon(); 


 PM25_AQI_Data data;
 if (! aqi.read(&data)) 
 {
  Serial.println("Could not read from AQI");
  delay(500);  
  return;
 }
  Serial.println("AQI reading success");

  Serial.print(F("\t\tPM 2.5: ")); Serial.print(data.pm25_standard);
  
 
  
  tft.setTextColor(TFT_ORANGE,TFT_BLACK);                                             
  uint16_t x = tft.width()/2;   // Find centre of screen
  tft.setTextDatum(MC_DATUM);   // Set datum to Middle Center
  int padding = tft.textWidth("8888", 4);
  tft.setTextPadding(padding);
  tft.drawNumber(data.pm25_standard, x, 60, 4);
 

  
  tft.setCursor(180, 65);
  tft.setTextSize(2); 
  tft.print("PM2.5");

  tft.setTextColor(TFT_SKYBLUE,TFT_BLACK);
  tft.setTextSize(2);

  tft.setCursor(50, 118);
  tft.print((bme.temperature-temperaure_offset),1);  

  tft.setCursor(55, 138);
  tft.println("÷C");

  tft.setCursor(165, 118);
  tft.print(bme.pressure / 100.0,1);

  tft.setCursor(168, 138);
  tft.println("hPa");

  tft.setTextColor(TFT_SKYBLUE,TFT_BLACK); 

  tft.setTextSize(2);
  tft.setCursor(50, 190); 
 
  tft.print(bme.humidity,1);
  bme.humidity;

  tft.setCursor(60, 210);
  tft.println("%");

  tft.setCursor(165, 190); 

  tft.print(bme.gas_resistance / 1000.0,1);

  tft.setCursor(168, 210);
  tft.println("KOhms");

  bme.readAltitude(SEALEVELPRESSURE_HPA);
  delay(50);  
}

 void WifiIcon() 
 {
  int32_t dbmicon = WiFi.RSSI();
   
  if (dbmicon <= -80 )  
  {
   tft.pushImage(225,0,16,16,wifi_1);
  } 

  else if (dbmicon <= -50 && dbmicon >= -79)
  {
   tft.pushImage(225,0,16,16,wifi_2);
  }

 else if (dbmicon >= -49)  
  {
   tft.pushImage(225,0,16,16,wifi_3);
  }
}
