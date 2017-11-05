#include "DHT_U.h"
#include <Wire.h>
#include "Adafruit_Sensor.h"
#include "Adafruit_TSL2561_U.h"

#define SDA D2
#define SCL D1
#define DHT_PORT D3
#define SOIL_PORT A0

int Humidnity, Tempurature, Soil, Light;
DHT dht(DHT_PORT, DHT22);
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);

void setup() {
  Serial.begin(9600);
  dht.begin();
  tsl.enableAutoRange(true);
  tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS); //13, 101, 402
}

void loop() {
  //Light Event
  sensors_event_t event;
  tsl.getEvent(&event);
  //Variable Update
  Humidnity = dht.readHumidity();
  Tempurature = dht.readTemperature();
  Soil = (1023-(analogRead(SOIL_PORT))/10.23);
  Light = event.light;
  

}
