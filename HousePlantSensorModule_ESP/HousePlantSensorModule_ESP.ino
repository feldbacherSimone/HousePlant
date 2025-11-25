#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_TSL2591.h"
#include <AHT20.h> 

#define SCL_PIN 22 
#define SDA_PIN 21
#define SOIL_PIN 2


#define uS_TO_S_FACTOR 1000000ULL  
#define TIME_TO_SLEEP 30


RTC_DATA_ATTR int bootCount = 0;

Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591); 
AHT20 aht20; 

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  delay(1000); 
  sensorSetup();
  //configureDeepSleep(); 
}

void configureDeepSleep(){
  bootCount++; 
  Serial.println("Boot number: " + String(bootCount));

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + " Seconds");



  Serial.println("Going to sleep now");
  Serial.flush();
  esp_deep_sleep_start();
}

void sensorSetup(){
   Wire.begin();
 // setup TSL2591 Light sensor 
  if (tsl.begin()) 
  {
    Serial.println(F("Found a TSL2591 sensor"));
  } 
  else 
  {
    Serial.println(F("No sensor found ... check your wiring?"));
    while (1);
  }
  configureSensor();
  displaySensorDetails();

  // setup AHT20 temp sensor
  if (aht20.begin() == false)
  {
    Serial.println("AHT20 not detected. Please check wiring. Freezing.");
    while(true);
  }
}

void loop() {
  // put your main code here, to run repeatedly:

  simpleRead(); 

}



void configureSensor(void)
{
  tsl.setGain(TSL2591_GAIN_MED); 
  tsl.setTiming(TSL2591_INTEGRATIONTIME_300MS);

  Serial.println(F("------------------------------------"));
  Serial.print  (F("Gain:         "));
  tsl2591Gain_t gain = tsl.getGain();
    switch(gain)
  {
    case TSL2591_GAIN_LOW:
      Serial.println(F("1x (Low)"));
      break;
    case TSL2591_GAIN_MED:
      Serial.println(F("25x (Medium)"));
      break;
    case TSL2591_GAIN_HIGH:
      Serial.println(F("428x (High)"));
      break;
    case TSL2591_GAIN_MAX:
      Serial.println(F("9876x (Max)"));
      break;
  }
}

void displaySensorDetails(void)
{
  sensor_t sensor;
  tsl.getSensor(&sensor);
  Serial.println(F("------------------------------------"));
  Serial.print  (F("Sensor:       ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:   ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:    ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:    ")); Serial.print(sensor.max_value); Serial.println(F(" lux"));
  Serial.print  (F("Min Value:    ")); Serial.print(sensor.min_value); Serial.println(F(" lux"));
  Serial.print  (F("Resolution:   ")); Serial.print(sensor.resolution, 4); Serial.println(F(" lux"));  
  Serial.println(F("------------------------------------"));
  Serial.println(F(""));
  delay(500);
}

void simpleRead(void)
{
  
  // Read Light
  uint16_t x = tsl.getLuminosity(TSL2591_VISIBLE);

  Serial.print(F("[ ")); Serial.print(millis()); Serial.print(F(" ms ] "));
  Serial.print(F("Luminosity: "));
  Serial.println(x, DEC);

  // Read Temp 

  float temp = aht20.getTemperature(); 
  float humidity = aht20.getHumidity();
  
  // Read Moisture

  int moisture = analogRead(SOIL_PIN);



}