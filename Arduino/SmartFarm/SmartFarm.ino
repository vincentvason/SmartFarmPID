// Library -------------------------------------------
#include <ESP8266WiFi.h>
#include "MicroGear.h"
#include "DHT_U.h"
#include <Wire.h>
#include "Adafruit_Sensor.h"
#include "Adafruit_TSL2561_U.h"
#include "CMMC_Interval.hpp"

// Port Setup -------------------------------------------
#define SDA D2
#define SCL D1
#define DHT_PORT D3
#define SOIL_PORT A0

// Key Setup ----------------------------------------------
#define KEY     "dKHySbTMVnV9A7g"
#define SECRET  "1gP2zHPiZy3XwTWZv3O6OxjXD"
#define APPID   "SmartFarmPID"
#define ALIAS   "SFP"

// Sensor Setup -------------------------------------------
int Humidnity, Tempurature, Soil, Light;
DHT dht(DHT_PORT, DHT22);
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);

// Client Setup --------------------------------------------
const char* ssid = "CE-ESL";
const char* password = "ceeslonly";

WiFiClient client;
CMMC_Interval timer;
MicroGear microgear(client);

// Message Arrival Function --------------------------------
void onMsghandler(char *topic, uint8_t* msg, unsigned int msglen) {
  msg[msglen] = '\0';
  String _msg = String((char*)msg);

  /* print message */
  Serial.print("Incoming message --> ");
  Serial.print(_msg);
  Serial.printf(" at [%lu] \r\n", millis());
}
void onFoundgear(char *attribute, uint8_t* msg, unsigned int msglen) {
    Serial.print("Found new member --> ");
    for (int i=0; i<msglen; i++)
        Serial.print((char)msg[i]);
    Serial.println();  
}
void onLostgear(char *attribute, uint8_t* msg, unsigned int msglen) {
    Serial.print("Lost member --> ");
    for (int i=0; i<msglen; i++)
        Serial.print((char)msg[i]);
    Serial.println();
}
// Publish Result ------------------------------------------
void Result(double h, double t, double s, double l){
    Serial.print("Publish...");
    Serial.print("\t");
    Serial.print(String(h));
    Serial.print("%H\t");
    Serial.print(String(t));
    Serial.print("C\t");
    Serial.print(String(s));
    Serial.print("%S\t");
    Serial.print(String(l));
    Serial.println("Lux\t");  
  
}

// Board Connected Function -------------------------------
void onConnected(char *attribute, uint8_t* msg, unsigned int msglen) {
    Serial.println("Connected to NETPIE...");
    /* Set the alias of this microgear ALIAS */
    microgear.setAlias(ALIAS);
}

void setup() {
  // Netpie Connected ---------------------------------------
  /* Call onMsghandler() when new message arraives */
  microgear.on(MESSAGE,onMsghandler);
  /* Call onFoundgear() when new gear appear */
  microgear.on(PRESENT,onFoundgear);
  /* Call onLostgear() when some gear goes offline */
  microgear.on(ABSENT,onLostgear);
  /* Call onConnected() when NETPIE connection is established */
  microgear.on(CONNECTED,onConnected);
  // Sensor Setup -------------------------------------------
  dht.begin();
  tsl.enableAutoRange(true);
  tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS); //13, 101, 402
  // Starting.. ---------------------------------------------
  Serial.begin(115200);
  Serial.println("Welcome.");
  // Wait for connection.. ----------------------------------
  /* Initial WIFI, this is just a basic method to configure WIFI on ESP8266.                       */
    /* You may want to use other method that is more complicated, but provide better user experience */
    if (WiFi.begin(ssid, password)) {
        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            Serial.print(".");
        }
    }
  // Result --------------------------------------------------
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Connecting to NETPIE.io");
  microgear.init(KEY,SECRET,ALIAS);
  microgear.connect(APPID);
  
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

  //MQTT Method
  /* Call this method regularly otherwise the connection may be lost */
  microgear.loop();
  /* To check if the microgear is still connected */
  if (microgear.connected()) {
    // call microgear.chat every 2 seconds
    timer.every_ms(2000, []() {
      Serial.printf("Publish at [%lu]\r\n", millis());
      Result(Humidnity, Tempurature, Soil, Light);
      microgear.chat("SFP/H", String(Humidnity));
      microgear.chat("SFP/T", String(Tempurature));
      microgear.chat("SFP/S", String(Soil));
      microgear.chat("SFP/L", String(Light));
    });

  }
  else {
    Serial.println("connection lost, reconnect...");
    microgear.connect(APPID);
    delay(2000);
  }

  
}
