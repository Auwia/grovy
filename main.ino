/* * Author: Massimo Manganiello * first development: 16/05/2018 * version: 1.0 */
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>
#include <Adafruit_MCP3008.h>
#include <Adafruit_MCP23017.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <OneWire.h>

// PHASE : (GERMINATION, VEGETABLE_LOW, VEGETABLE_HIGH, FLOWERING_LOW, FLOWERING_HIGH)
const char* PHASE = "GERMINATION";
// LIGHT : (ON, OFF)
const char* LIGHT = "ON";
// WATER : (ON, OFF)
const char* WATER = "OFF";

// WIFI const
char* ssid = "UPCA9E82C2";
const char* wifi_password = "tp3Ya2mkhztk";
WiFiClient client;

// MYSQL
char* host = "e73173-mysql.services.easyname.eu";
IPAddress server_addr(185, 51, 11, 3);
char user[] = "u112031db1";
char password[] = "4i340So";
MySQL_Connection conn((Client *)&client);

// MCP3008
Adafruit_MCP3008 adc;
#define CLOCK_PIN D5
#define MISO_PIN D6
#define MOSI_PIN D7
#define CS_PIN D8

// RANGE SENSOR
const int trigPin = 2; //D4
const int echoPin = 0; //D3
long duration;
int distance;

// TEMPERATURE
#define ONE_WIRE_BUS 10 //Pin to which is attached a temperature sensor 
#define ONE_WIRE_MAX_DEV 1 //The maximum number of devices 
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// TIME - 5 minutes timer
int cont = 1;

// MCP23017 PIN
Adafruit_MCP23017 mcp0;
const int FAN1IN = 0;
const int FAN2IN = 1;
const int FAN1OUT = 2;
const int FAN2OUT = 3;
const int FANLED = 4;
const int CO2 = 5;
const int LED_WARM_WHITE = 6;
const int LED_FULL_SPECTRUM_UV = 7;
const int LED_DEEP_RED_1 = 8;
const int LED_DEEP_RED_2 = 9;
const int LED_BRIGHT_RED = 10;
const int LED_BRIGHT_BLUE_1 = 11;
const int LED_BRIGHT_BLUE_2 = 12;
const int LED_ROYAL_BLUE = 13;
const int WATER_PUMP = 14;
const int HEATER = 15;
const int PELTIER = 16;

void setup() {
  Serial.begin(115200);
  delay(10);

  // WIFI
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

  // TEMPERATURE
  sensors.begin();

  // MCP23017
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

  // MYSQL
  if (conn.connect(server_addr, 3306, user, password)) {
    delay(1000);
  } else Serial.println("Connection failed.");

  // MCP3008
  Serial.print("MCP3008 init...");
  adc.begin(CLOCK_PIN, MOSI_PIN, MISO_PIN, CS_PIN);
  Serial.println("OK");

  // RANGE SENSOR
  Serial.print("HC-SR04 UltraSonic init...");
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  Serial.println("OK");
}
void loop() {
  // MYSQL
  MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);

  // TIME TIMESTAMP
  String timestamp;
  HTTPClient http;
  http.begin("http://weinzuhause.altervista.org/ws/getDateTime.php");
  int httpCode = http.GET();
  if (httpCode > 0) {
    timestamp = http.getString();
    if (timestamp.substring(11, 12).toInt() >= 8 || timestamp.substring(11, 12).toInt() <= 19) {
      LIGHT = "ON";
    } else {
      LIGHT = "OFF";
    }
    if ((timestamp.substring(11, 12).toInt() == 7 &&
         timestamp.substring(14, 15).toInt() == 45) ||
        (timestamp.substring(11, 12).toInt() == 19 && timestamp.substring(14, 15).toInt() == 45 ))
    {
      WATER = "ON";
    } else {
      WATER = "OFF";
    }
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

  // RANGE SENSOR
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
  distance = duration * 0.034 / 2;
  // Prints the distance on the Serial Monitor
  Serial.print(timestamp + ": Distance: ");
  Serial.println(distance);

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
    cur_mem->execute(UPDATE_SQL.c_str());
    delete cur_mem;
  }

  Serial.println(timestamp + ": Recording data of moisture.");
  String INSERT_SQL = (String)"insert into u112031db1.mis_soil (timestamp, area, moisture) values (convert_tz(now(),@@session.time_zone,\'+02:00\'), \'Moisture\', " + adc.readADC(1) + ")";
  cur_mem = new MySQL_Cursor(&conn);
  cur_mem->execute(INSERT_SQL.c_str());
  delete cur_mem;

  Serial.println(timestamp + ": Recording data of light spectrum.");
  String UPDATE_SQL = (String)"update u112031db1.probe set status=" + adc.readADC(2) + " where name='light_spectrum'";
  cur_mem = new MySQL_Cursor(&conn);
  cur_mem->execute(UPDATE_SQL.c_str());
  delete cur_mem;

  if (cont == 5) {
    Serial.println(timestamp + ": Recording distance.");
    String INSERT_SQL = (String)"insert into u112031db1.mis_distance (timestamp, area, distance) values (convert_tz(now(),@@session.time_zone,\'+02:00\'), \'Distance\', " + distance + ")";
    cur_mem = new MySQL_Cursor(&conn);
    cur_mem->execute(INSERT_SQL.c_str());
    delete cur_mem;
  }

  if (LIGHT == "ON") {
    if (PHASE == "GERMINATION") {
      mcp0.digitalWrite(LED_WARM_WHITE, HIGH);
      mcp0.digitalWrite(LED_FULL_SPECTRUM_UV, HIGH);
      mcp0.digitalWrite(LED_ROYAL_BLUE, HIGH);
      mcp0.digitalWrite(LED_BRIGHT_BLUE_1, LOW);
      mcp0.digitalWrite(LED_BRIGHT_BLUE_2, LOW);
      mcp0.digitalWrite(LED_BRIGHT_RED, LOW);
      mcp0.digitalWrite(LED_DEEP_RED_1, LOW);
      mcp0.digitalWrite(LED_DEEP_RED_2, LOW);
    }
    if (PHASE == "VEGETABLE_LOW") {
      mcp0.digitalWrite(LED_WARM_WHITE, LOW);
      mcp0.digitalWrite(LED_FULL_SPECTRUM_UV, HIGH);
      mcp0.digitalWrite(LED_ROYAL_BLUE, HIGH);
      mcp0.digitalWrite(LED_BRIGHT_BLUE_1, LOW);
      mcp0.digitalWrite(LED_BRIGHT_BLUE_2, LOW);
      mcp0.digitalWrite(LED_BRIGHT_RED, LOW);
      mcp0.digitalWrite(LED_DEEP_RED_1, LOW);
      mcp0.digitalWrite(LED_DEEP_RED_2, LOW);
    }
    if (PHASE == "VEGETABLE_HIGH") {
      mcp0.digitalWrite(LED_WARM_WHITE, LOW);
      mcp0.digitalWrite(LED_FULL_SPECTRUM_UV, HIGH);
      mcp0.digitalWrite(LED_ROYAL_BLUE, HIGH);
      mcp0.digitalWrite(LED_BRIGHT_BLUE_1, HIGH);
      mcp0.digitalWrite(LED_BRIGHT_BLUE_2, HIGH);
      mcp0.digitalWrite(LED_BRIGHT_RED, LOW);
      mcp0.digitalWrite(LED_DEEP_RED_1, LOW);
      mcp0.digitalWrite(LED_DEEP_RED_2, LOW);
    }
    if (PHASE == "FLOWERING_LOW") {
      mcp0.digitalWrite(LED_WARM_WHITE, LOW);
      mcp0.digitalWrite(LED_FULL_SPECTRUM_UV, HIGH);
      mcp0.digitalWrite(LED_ROYAL_BLUE, LOW);
      mcp0.digitalWrite(LED_BRIGHT_BLUE_1, LOW);
      mcp0.digitalWrite(LED_BRIGHT_BLUE_2, LOW);
      mcp0.digitalWrite(LED_BRIGHT_RED, HIGH);
      mcp0.digitalWrite(LED_DEEP_RED_1, HIGH);
      mcp0.digitalWrite(LED_DEEP_RED_2, HIGH);
    }
    if (PHASE == "FLOWERING_HIGH") {
      mcp0.digitalWrite(LED_WARM_WHITE, LOW);
      mcp0.digitalWrite(LED_FULL_SPECTRUM_UV, HIGH);
      mcp0.digitalWrite(LED_ROYAL_BLUE, LOW);
      mcp0.digitalWrite(LED_BRIGHT_BLUE_1, LOW);
      mcp0.digitalWrite(LED_BRIGHT_BLUE_2, LOW);
      mcp0.digitalWrite(LED_BRIGHT_RED, LOW);
      mcp0.digitalWrite(LED_DEEP_RED_1, HIGH);
      mcp0.digitalWrite(LED_DEEP_RED_2, HIGH);
    }
  } else {
    mcp0.digitalWrite(LED_WARM_WHITE, LOW);
    mcp0.digitalWrite(LED_FULL_SPECTRUM_UV, LOW);
    mcp0.digitalWrite(LED_ROYAL_BLUE, LOW);
    mcp0.digitalWrite(LED_BRIGHT_BLUE_1, LOW);
    mcp0.digitalWrite(LED_BRIGHT_BLUE_2, LOW);
    mcp0.digitalWrite(LED_BRIGHT_RED, LOW);
    mcp0.digitalWrite(LED_DEEP_RED_1, LOW);
    mcp0.digitalWrite(LED_DEEP_RED_2, LOW);
  }

  if (WATER == "ON") {
    mcp0.digitalWrite(WATER_PUMP, HIGH);
  } else {
    mcp0.digitalWrite(WATER_PUMP, LOW);
  }

  /*
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
  */

  // 5 minutes
  if (cont == 5) {
    cont = 0;
  }

  delay(60000);

  // 5 minutes
  cont = cont + 1;
}
