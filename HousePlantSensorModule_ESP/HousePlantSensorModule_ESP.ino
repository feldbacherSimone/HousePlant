#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_TSL2591.h"
#include <AHT20.h> 
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define SCL_PIN 22 
#define SDA_PIN 21
#define SOIL_PIN 32
#define POWER_PIN 27

#define uS_TO_S_FACTOR 1000000ULL  
#define TIME_TO_SLEEP 15


RTC_DATA_ATTR int bootCount = 0;

Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591); 
AHT20 aht20; 
WiFiMulti wifiMulti; 


const char*  ssid = "SalamiLid";
const char* password = "uwuuwusb";
const char* API_KEY = "V2ZZ64D951T0UQC2"; 

void setup() {
  // put your setup code here, to run once:
  pinMode(POWER_PIN, OUTPUT);
  digitalWrite(POWER_PIN, LOW);  // default OFF


  Serial.begin(115200);


  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  wifiMulti.addAP(ssid, password); 
  sensorSetup();

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

  digitalWrite(POWER_PIN, HIGH);
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
  //displaySensorDetails();

  // setup AHT20 temp sensor
  if (aht20.begin() == false)
  {
    Serial.println("AHT20 not detected. Please check wiring. Freezing.");
    while(true);
  }

}

void loop() {

  int soilValue = analogRead(SOIL_PIN); 
  float temperature = aht20.getTemperature(); 
  float humidity = aht20.getHumidity(); 
  int light = ReadLumin();  
  digitalWrite(POWER_PIN, LOW);

  int wifiStatus = wifiMulti.run();
  if (wifiStatus == WL_CONNECTED) {
    // --- Local Server ---
    /*
    
    HTTPClient http;
    http.begin("http://10.98.224.62:3000/soil");
    http.addHeader("Content-Type", "application/json"); 

    StaticJsonDocument<200> localDoc;
    localDoc["readingID"] = "Test";
    localDoc["temp_value"] = temperature;
    localDoc["light_value"] = light;
    localDoc["humidity_value"] = humidity;
    localDoc["soil_value"] = soilValue;

    delay(10);
    digitalWrite(POWER_PIN, LOW);

    String localBody;
    serializeJson(localDoc, localBody);
    int httpCode = http.POST(localBody);
    if(httpCode > 0){
      Serial.printf("[Local] POST... code: %d\n", httpCode);
      Serial.println(http.getString());
    } else {
      Serial.printf("[Local] POST failed: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();

    */
    // --- ThingSpeak ---
    HTTPClient tsHttp;
    tsHttp.begin("https://api.thingspeak.com/update.json");
    tsHttp.addHeader("Content-Type", "application/json"); 

    StaticJsonDocument<200> tsDoc;
    tsDoc["api_key"] = API_KEY;
    tsDoc["field1"] = temperature;
    tsDoc["field2"] = light;
    tsDoc["field3"] = humidity;
    tsDoc["field4"] = soilValue;

    String tsBody;
    serializeJson(tsDoc, tsBody);
    int tsCode = tsHttp.POST(tsBody);
    if(tsCode > 0){
      Serial.printf("[ThingSpeak] POST... code: %d\n", tsCode);
      Serial.println(tsHttp.getString());
    } else {
      Serial.printf("[ThingSpeak] POST failed: %s\n", tsHttp.errorToString(tsCode).c_str());
    }
    tsHttp.end();
  }
  configureDeepSleep(); // Comment out for continous connection
  delay(10000);
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

int ReadLumin()
{
    // Read Light
  uint16_t x = tsl.getLuminosity(TSL2591_VISIBLE);

  Serial.print(F("[ ")); Serial.print(millis()); Serial.print(F(" ms ] "));
  Serial.print(F("Luminosity: "));
  Serial.println(x, DEC);

  return x; 
}



