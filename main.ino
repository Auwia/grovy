/*
 *  This sketch sends data via HTTP GET requests to data.sparkfun.com service.
 *
 *  You need to get streamId and privateKey at data.sparkfun.com and paste them
 *  below. Or just customize this script to talk to other HTTP servers.
 *
 */

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>
#include <Wire.h> 
#include <Adafruit_MCP3008.h>
#include <Adafruit_MCP23017.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// DHT22 TEMPERATURE HUMIDITY SENSOR
#include "DHT.h"

const char* ssid = "UPCA9E82C2";
const char* wifi_password = "tp3Ya2mkhztk";

const char* host = "e73173-mysql.services.easyname.eu";
char server[] = "www.irpiniaimport.ch/ws/getIPfromHostname.php";
IPAddress server_addr(185, 51, 11, 3);
char user[] = "u112031db1";
char password[] = "4i340So";

WiFiClient client;

// MYSQL
MySQL_Connection conn((Client *)&client);

Adafruit_MCP3008 adc;
#define CLOCK_PIN D5
#define MISO_PIN D6
#define MOSI_PIN D7
#define CS_PIN D8

const int trigPin = 2;  //D4
const int echoPin = 0;  //D3
long duration;
int distance;

// MCP23017
Adafruit_MCP23017 mcp0;

// TEMPERATURE
#define ONE_WIRE_BUS 10 //Pin to which is attached a temperature sensor
#define ONE_WIRE_MAX_DEV 1 //The maximum number of devices
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// 5 minutes
int cont = 1;

void setup() {
  // TEMPERATURE
   sensors.begin();

  Serial.begin(115200);
  delay(10);

  mcp0.begin(0);
  mcp0.pinMode(0, OUTPUT);
  mcp0.pinMode(1, OUTPUT);
  mcp0.pinMode(2, OUTPUT);
  mcp0.pinMode(3, OUTPUT);
  mcp0.pinMode(4, OUTPUT);
  mcp0.pinMode(5, OUTPUT);
  mcp0.pinMode(6, OUTPUT);
  mcp0.pinMode(7, OUTPUT);
  mcp0.pinMode(8, OUTPUT);
  mcp0.pinMode(9, OUTPUT);
  mcp0.pinMode(10, OUTPUT);
  mcp0.pinMode(11, OUTPUT);
  mcp0.pinMode(12, OUTPUT);
  mcp0.pinMode(13, OUTPUT);
  mcp0.pinMode(14, OUTPUT);
  mcp0.pinMode(15, OUTPUT);

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, wifi_password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Starting connection to server...");
  if (client.connect(server, 80)) {
    Serial.println("connected to server");
    // Make a HTTP request:
    Serial.println("GET /search?q=arduino HTTP/1.1");
    Serial.println("Host: www.google.com");
    Serial.println("Connection: close");
    Serial.println();

    if (client.available()) {
      char c = client.read();
      Serial.write(c);
    }
  }  

  if (conn.connect(server_addr, 3306, user, password)) {
    delay(1000);
  }
  else Serial.println("Connection failed.");

  Serial.print("MCP3008 init...");
  adc.begin(CLOCK_PIN, MOSI_PIN, MISO_PIN, CS_PIN);
  Serial.println("OK");

  Serial.print("HC-SR04 UltraSonic init...");
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  Serial.println("OK");

  // TIME
  Serial.print("TIME LIBRARY init...");

  Serial.println("OK");
}

int value = 0;

void loop() {
  MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
  // TIME TIMESTAMP
  String timestamp;
  HTTPClient http; 
  http.begin("http://weinzuhause.altervista.org/ws/getDateTime.php");  //Specify request destination
  int httpCode = http.GET();
  if (httpCode > 0) {
    timestamp = http.getString();
    Serial.println(timestamp);
  } else {
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();
  
  // TEMPERATURE
  Serial.print("Requesting temperatures...");
  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.println("DONE");

  Serial.print("Temperature for the device 1 (index 0) is: ");
  Serial.println(sensors.getTempCByIndex(0));

  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  distance= duration*0.034/2;
  // Prints the distance on the Serial Monitor
  Serial.print(timestamp + ": Distance: ");
  Serial.println(distance);
     
  /*
  int val = analogRead(A0); // read input value
  Serial.print("ESP8266_ADC0: ");
  Serial.println(val);
  */
  Serial.println((String)timestamp + ": MCP3008_ADC_0: " + adc.readADC(0)); // water level
  Serial.println((String)timestamp + ": MCP3008_ADC_1: " + adc.readADC(1)); // moisture sensor
  Serial.println((String)timestamp + ": MCP3008_ADC_2: " + adc.readADC(2)); // light spectrum
  Serial.println((String)timestamp + ": MCP3008_ADC_3: " + adc.readADC(3));
  Serial.println((String)timestamp + ": MCP3008_ADC_4: " + adc.readADC(4));
  Serial.println((String)timestamp + ": MCP3008_ADC_5: " + adc.readADC(5));
  Serial.println((String)timestamp + ": MCP3008_ADC_6: " + adc.readADC(6));
  Serial.println((String)timestamp + ": MCP3008_ADC_7: " + adc.readADC(7));

  if (cont == 5) {
    Serial.println(timestamp + ": Recording data of water level.");
    String UPDATE_SQL = (String)"update u112031db1.probe set status=" + adc.readADC(0) + " where name='water_level'";
    cur_mem = new MySQL_Cursor(&conn);
  // Execute the query
    cur_mem->execute(UPDATE_SQL.c_str());
    // Note: since there are no results, we do not need to read any data
    // Deleting the cursor also frees up memory used
    delete cur_mem;
  }
  
  Serial.println(timestamp + ": Recording data of moisture.");
  String INSERT_SQL = (String)"insert into u112031db1.mis_soil (timestamp, area, moisture) values (convert_tz(now(),@@session.time_zone,\'+02:00\'), \'Moisture\', " + adc.readADC(1) + ")";
  cur_mem = new MySQL_Cursor(&conn);
  // Execute the query
  cur_mem->execute(INSERT_SQL.c_str());
  // Note: since there are no results, we do not need to read any data
  // Deleting the cursor also frees up memory used
  delete cur_mem;

  Serial.println(timestamp + ": Recording data of light spectrum.");
  String UPDATE_SQL = (String)"update u112031db1.probe set status=" + adc.readADC(2) + " where name='light_spectrum'";
  cur_mem = new MySQL_Cursor(&conn);
  // Execute the query
  cur_mem->execute(UPDATE_SQL.c_str());
  // Note: since there are no results, we do not need to read any data
  // Deleting the cursor also frees up memory used
  delete cur_mem;

  if (cont == 5) {
    Serial.println(timestamp + ": Recording distance.");
    String INSERT_SQL = (String)"insert into u112031db1.mis_distance (timestamp, area, distance) values (convert_tz(now(),@@session.time_zone,\'+02:00\'), \'Distance\', " + distance + ")";
  cur_mem = new MySQL_Cursor(&conn);
    // Execute the query
    cur_mem->execute(INSERT_SQL.c_str());
    // Note: since there are no results, we do not need to read any data
    // Deleting the cursor also frees up memory used
    delete cur_mem;
  }
  

  mcp0.digitalWrite(0, HIGH);
  delay(500);
  mcp0.digitalWrite(0, LOW);

  mcp0.digitalWrite(1, HIGH);
  delay(500);
  mcp0.digitalWrite(1, LOW);

  mcp0.digitalWrite(2, HIGH);
  delay(500);
  mcp0.digitalWrite(2, LOW);

  mcp0.digitalWrite(3, HIGH);
  delay(500);
  mcp0.digitalWrite(3, LOW);

  mcp0.digitalWrite(4, HIGH);
  delay(500);
  mcp0.digitalWrite(4, LOW);

  mcp0.digitalWrite(5, HIGH);
  delay(500);
  mcp0.digitalWrite(5, LOW);

  mcp0.digitalWrite(6, HIGH);
  delay(500);
  mcp0.digitalWrite(6, LOW);

  Serial.println((String)timestamp + ": MCP23017_PIN_8_READ: " + mcp0.digitalRead(8));
  Serial.println((String)timestamp + ": MCP23017_PIN_9_READ: " + mcp0.digitalRead(9));
  
  // 5 minutes
  if (cont == 5) {
    cont = 0;
  }
  
  delay(60000);

  // 5 minutes
  cont = cont + 1;
  
}
