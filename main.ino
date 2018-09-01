/* * Author: Massimo Manganiello * */
/* 16/05/2018: first development. * version: 1.0 */
/* 17/06/2018: new core / feature. * version: 2.0 */
/* 21/06/2018: no mysql connection. * version: 3.0 */
/* 28/07/2018: 16ch. relay board. driver MCP23017 * version: 3.1 */
/* 20/08/2018: clean up * version: 3.2 */
/* 28/08/2018: blink blue led when esp8266 is working, upload from remote OTA * version: 3.3 */
/* 01/09/2018: MQTT reconnect in case of lost connection, remote debug by telnet * version: 3.4 */

#include <OneWire.h>
#include <Adafruit_MCP3008.h>
#include <Adafruit_MCP23017.h>
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <PZEM004T.h>
#include <RemoteDebug.h>

// PZEM004T
PZEM004T pzem(&Serial1);
IPAddress ip(192,168,1,1);

// PHASE : (GERMINATION, VEGETABLE_LOW, VEGETABLE_HIGH, FLOWERING_LOW, FLOWERING_HIGH)
const char* PHASE = "GERMINATION";
// LIGHT : (ON, OFF)
const char* LIGHT = "ON";

// WIFI
const char* ssid = "UPCA9E82C2";
const char* password_wifi =  "tp3Ya2mkhztk";
WiFiClient clientWIFI;
WiFiServer server(23);

// MQTT client
const char* mqttServer = "192.168.0.178";
const int mqttPort = 1883;
const char* mqttUser = "";
const char* mqttPassword = "";
PubSubClient clientMQTT(clientWIFI);

// MCP3008
Adafruit_MCP3008 adc;
#define CLOCK_PIN D5
#define MISO_PIN D6
#define MOSI_PIN D7
#define CS_PIN D8

// RANGE SENSOR
const int trigPin = 0; //D4
const int echoPin = 0; //D3
long duration;
int distance;

// TEMPERATURE
#define ONE_WIRE_BUS 15 //Pin to which is attached a temperature sensor 
#define ONE_WIRE_MAX_DEV 1 //The maximum number of devices 
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// MCP23017 PIN
Adafruit_MCP23017 mcp0;
const int FAN_IN = 0;
const int FAN_OUT = 1;
const int COOL_LAMP = 2;
const int CO2 = 3;
const int EMPTY = 4;
const int PELTIER = 5;
const int WATER_PUMP = 6;
const int DEHUMIDIFIER = 7;
const int LED_FULL_SPECTRUM_UV = 8;
const int LED_BRIGHT_BLUE = 9; 
const int LED_DEEP_RED = 10;
const int LED_BRIGHT_RED = 11;
const int LED_ROYAL_BLUE = 12;
const int LED_WARM_WHITE = 13;
const int HEATER_FAN = 14;
const int HEATER = 15;
const int FAN_IN_PWM = 16; // only for new feature PWM

// BLUE LED
int LED = 2;

// TIMESTAMP
String timestamp;

// REMOTE UPDATE OTA
const char* host = "esp8266-webupdate";
ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

// TELNET REMOTE DEBUG
bool RemoteSerial = true; //true = Remote and local serial, false = local serial only
RemoteDebug Debug;
#define MAX_SRV_CLIENTS 1
WiFiClient serverClients[MAX_SRV_CLIENTS];

void setup() {
  Serial.begin(115200);

  // WIFI
  WiFi.begin(ssid, password_wifi); 
  Serial.print("Connecting to WiFi.");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("ok.");

  // MQTT client
  clientMQTT.setServer(mqttServer, mqttPort);
  clientMQTT.setCallback(callback);

  // REMOTE UPDATE OTA
  MDNS.begin(host);
  httpUpdater.setup(&httpServer);
  httpServer.begin();
  MDNS.addService("http", "tcp", 80);
  Serial.printf("HTTPUpdateServer ready! Open http://%d.local/update in your browser\n", host);

  // TELNET REMOTE DEBUG
  MDNS.addService("telnet", "tcp", 23);
  Debug.begin(host); 
  Debug.setSerialEnabled(true);
  Debug.setResetCmdEnabled(true); // Enable the reset command
  rdebugIln("TELNET REMOTE DEBUG...ok!");
  
  // PZEM004T
  pzem.setAddress(ip);
  
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
  mcp0.digitalWrite(FAN_IN, HIGH);
  mcp0.digitalWrite(FAN_OUT, HIGH);
  mcp0.digitalWrite(COOL_LAMP, HIGH);
  mcp0.digitalWrite(CO2, HIGH);
  mcp0.digitalWrite(HEATER, HIGH);
  mcp0.digitalWrite(PELTIER, HIGH);
  mcp0.digitalWrite(WATER_PUMP, HIGH);
  mcp0.digitalWrite(DEHUMIDIFIER, HIGH);
  mcp0.digitalWrite(LED_WARM_WHITE, HIGH);
  mcp0.digitalWrite(LED_FULL_SPECTRUM_UV, HIGH);
  mcp0.digitalWrite(LED_DEEP_RED, HIGH);
  mcp0.digitalWrite(LED_BRIGHT_RED, HIGH);
  mcp0.digitalWrite(LED_BRIGHT_BLUE, HIGH);
  mcp0.digitalWrite(LED_ROYAL_BLUE, HIGH);
  mcp0.digitalWrite(EMPTY, HIGH);
  mcp0.digitalWrite(HEATER_FAN, HIGH);
      
  // MCP3008
  Serial.print("MCP3008 init...");
  rdebugI("MCP3008 init...");
  adc.begin(CLOCK_PIN, MOSI_PIN, MISO_PIN, CS_PIN);
  Serial.println("OK");
  rdebugIln("OK");

  // RANGE SENSOR
  Serial.print("HC-SR04 UltraSonic init...");
  rdebugI("HC-SR04 UltraSonic init...");
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  Serial.println("OK");
  rdebugIln("OK");

  delay(1500);
}
 
void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  Serial.print("Message arrived in topic: ");
  rdebugD("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message:");
  rdebugDln("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    message += (char)payload[i];
  }
  Serial.println("");
  delay(1000);

  if (String(topic).equals("dehumidifier")) {
    if(message.equals("1")) {
      Serial.println("DEHUMIDIFIER->ON");
      rdebugDln("DEHUMIDIFIER->ON");
      mcp0.digitalWrite(DEHUMIDIFIER, LOW);
    } else {
      Serial.println("DEHUMIDIFIER->OFF");
      rdebugDln("DEHUMIDIFIER->OFF");
      mcp0.digitalWrite(DEHUMIDIFIER, HIGH);      
    }
  }
  
  if (String(topic).equals("peltier")) {
    if(message.equals("1")) {
      Serial.println("PELTIER->ON");
      rdebugDln("PELTIER->ON");
      mcp0.digitalWrite(PELTIER, LOW);
    } else {
      Serial.println("PELTIER->OFF");
      rdebugDln("PELTIER->OFF");
      mcp0.digitalWrite(PELTIER, HIGH);      
    }
  }
  
  if (String(topic).equals("heater")) {
    if(message.equals("1")) {
      Serial.println("HEATER->ON");
      rdebugDln("HEATER->ON");
      mcp0.digitalWrite(HEATER, LOW);
    } else {
      Serial.println("HEATER->OFF");
      rdebugDln("HEATER->OFF");
      mcp0.digitalWrite(HEATER, HIGH);      
    }
  }

  if (String(topic).equals("co2")) {
    if(message.equals("1")) {
      Serial.println("CO2->ON");
      rdebugDln("CO2->ON");
      mcp0.digitalWrite(CO2, LOW);
    } else {
      Serial.println("CO2->OFF");
      rdebugDln("CO2->OFF");
      mcp0.digitalWrite(CO2, HIGH);      
    }
  }
  
  if (String(topic).equals("coolLamp")) {
    if(message.equals("1")) {
      Serial.println("COOL LAMP->ON");
      rdebugDln("COOL LAMP->ON");
      mcp0.digitalWrite(COOL_LAMP, LOW);
    } else {
      Serial.println("COOL_LAMP->OFF");
      rdebugDln("COOL_LAMP->OFF");
      mcp0.digitalWrite(COOL_LAMP, HIGH);      
    }
  }

  if (String(topic).equals("fanOut")) {
    if(message.equals("1")) {
      Serial.println("FAN OUT->ON");
      rdebugDln("FAN OUT->ON");
      mcp0.digitalWrite(FAN_OUT, LOW);
    } else {
      Serial.println("FAN OUT->OFF");
      rdebugDln("FAN OUT->OFF");
      mcp0.digitalWrite(FAN_OUT, HIGH);      
    }
  }
 
  if (String(topic).equals("fanIn")) {
    setFanIn((char)payload[0]);
  }
  
  if (String(topic).equals("water_pump")) {
    if(message.equals("1")) {
      Serial.println("WATER->ON");
      rdebugDln("WATER->ON");
      mcp0.digitalWrite(WATER_PUMP, LOW);
    } else {
      Serial.println("WATER->OFF");
      rdebugDln("WATER->OFF");
      mcp0.digitalWrite(WATER_PUMP, HIGH);      
    }
  }

  
  if (String(topic).equals("built-in_led")) {
    // BLUE LED
    pinMode(LED, OUTPUT);
    digitalWrite(LED, HIGH);
    if(message.equals("1")) {
      Serial.println("BUILT-IN LED->ON");
      rdebugDln("BUILT-IN LED->ON");
      digitalWrite(LED, LOW);
    } else {
      Serial.println("BUILT-IN LED->OFF");
      rdebugDln("BUILT-IN LED->OFF");
      digitalWrite(LED, HIGH);      
    }
  }

  if (String(topic).equals("testSingle")) {
    Serial.println("TEST PIN: " + message + " ->ON");
    rdebugDln("TEST PIN: %d ->ON", message.c_str());
    mcp0.digitalWrite(message.toInt(), LOW);
  }
  
  if (String(topic).equals("testSingleOff")) {
    Serial.println("TEST PIN: " + message + " ->OFF");
    rdebugDln("TEST PIN: %d ->OFF", message.c_str());
    mcp0.digitalWrite(message.toInt(), HIGH);
  }

  if (String(topic).equals("test")) {
    if(message.equals("1")) {
      Serial.println("TEST->ON");
      rdebugDln("TEST->ON");
      mcp0.digitalWrite(FAN_IN, LOW);
      mcp0.digitalWrite(FAN_OUT, LOW);
      mcp0.digitalWrite(COOL_LAMP, LOW);
      mcp0.digitalWrite(CO2, LOW);
      mcp0.digitalWrite(HEATER, LOW);
      mcp0.digitalWrite(PELTIER, LOW);
      mcp0.digitalWrite(WATER_PUMP, LOW);
      mcp0.digitalWrite(DEHUMIDIFIER, LOW);
      mcp0.digitalWrite(LED_WARM_WHITE, LOW);
      mcp0.digitalWrite(LED_FULL_SPECTRUM_UV, LOW);
      mcp0.digitalWrite(LED_DEEP_RED, LOW);
      mcp0.digitalWrite(LED_BRIGHT_RED, LOW);
      mcp0.digitalWrite(LED_BRIGHT_BLUE, LOW);
      mcp0.digitalWrite(LED_ROYAL_BLUE, LOW);
      mcp0.digitalWrite(EMPTY, LOW);
      mcp0.digitalWrite(HEATER_FAN, LOW);
      Serial.println("TEST->OFF");
      rdebugDln("TEST->OFF");
    } else {
      Serial.println("TEST->ON");
      rdebugDln("TEST->ON");
      mcp0.digitalWrite(FAN_IN, HIGH);
      mcp0.digitalWrite(FAN_OUT, HIGH);
      mcp0.digitalWrite(COOL_LAMP, HIGH);
      mcp0.digitalWrite(CO2, HIGH);
      mcp0.digitalWrite(HEATER, HIGH);
      mcp0.digitalWrite(PELTIER, HIGH);
      mcp0.digitalWrite(WATER_PUMP, HIGH);
      mcp0.digitalWrite(DEHUMIDIFIER, HIGH);
      mcp0.digitalWrite(LED_WARM_WHITE, HIGH);
      mcp0.digitalWrite(LED_FULL_SPECTRUM_UV, HIGH);
      mcp0.digitalWrite(LED_DEEP_RED, HIGH);
      mcp0.digitalWrite(LED_BRIGHT_RED, HIGH);
      mcp0.digitalWrite(LED_BRIGHT_BLUE, HIGH);
      mcp0.digitalWrite(LED_ROYAL_BLUE, HIGH);
      mcp0.digitalWrite(EMPTY, HIGH);
      mcp0.digitalWrite(HEATER_FAN, HIGH);
      Serial.println("TEST->OFF");      
      rdebugDln("TEST->OFF");
    }     
  }
     
  if (String(topic).equals("phase")) {
    switch ((char)payload[0]) {
      // PHASE : (0: GERMINATION, 1: VEGETABLE_LOW, 2: VEGETABLE_HIGH, 3: FLOWERING_LOW, 4: FLOWERING_HIGH)
      case '0':
        Serial.println("PHASE=GERMINATION");
        rdebugDln("PHASE=GERMINATION");
        PHASE = "GERMINATION";
        break;
      case '1':
        Serial.println("PHASE=VEGETABLE_LOW");
        rdebugDln("PHASE=VEGETABLE_LOW");
        PHASE = "VEGETABLE_LOW";
        break;
      case '2':
        Serial.println("PHASE=VEGETABLE_HIGH");
        rdebugDln("PHASE=VEGETABLE_HIGH");
        PHASE = "VEGETABLE_HIGH";
        break;
      case '3':
        Serial.println("PHASE=FLOWERING_LOW");
        rdebugDln("PHASE=FLOWERING_LOW");
        PHASE = "FLOWERING_LOW";
        break;
      case '4':
        Serial.println("PHASE=FLOWERING_HIGH");
        rdebugDln("PHASE=FLOWERING_HIGH");
        PHASE = "FLOWERING_HIGH";
        break;
    }
  }
  if (String(topic).equals("distance")) {
    Serial.print("distance: ");
    rdebugD("distance: ");
    float distance = getDistance();
    Serial.println(distance);
    rdebugDln("%d", distance);
  }
  if (String(topic).equals("water_level")) {
    Serial.print("water_level: ");
    rdebugD("water_level: ");
    float water_level = getWaterLevel();
    Serial.println(water_level);
    rdebugDln("%d", water_level);
  }
  if (String(topic).equals("light_spectrum")) {
    Serial.print("light_spectrum: ");
    rdebugD("light_spectrum: ");
    int light_spectrum = getLightSpectrum();
    Serial.println(light_spectrum);
    rdebugDln("%d", light_spectrum);
  }
  if (String(topic).equals("temperature")) {
    Serial.print("temperature: ");
    rdebugD("temperature: ");
    float temperature = getTemperature();
    Serial.println(temperature);
    rdebugDln("%d", temperature);
  }
  if (String(topic).equals("moisture")) {
    Serial.print("moisture: ");
    rdebugD("moisture: ");
    int moisture = getMoisture();
    Serial.println(moisture);
    rdebugDln("%d", moisture);
  }
  if (String(topic).equals("energy")) {
    Serial.print("Energy: ");
    String energy_res = getEnergy();
    Serial.println(energy_res);
  }
  Serial.println();
  Serial.println("-----------------------");
  rdebugDln();
  rdebugDln("-----------------------");
}

void setFanIn(int duty) {
   switch (duty) {
    case '1':
      Serial.print("FAN INPUT -> ON at");
      mcp0.digitalWrite(FAN_IN, LOW);
      Serial.println(" 10 % PWM");   
      analogWrite(FAN_IN_PWM, 102);
      break;
    case '2':
      Serial.print("FAN INPUT -> ON at");
      mcp0.digitalWrite(FAN_IN, LOW);
      Serial.println(" 20 % PWM");
      analogWrite(FAN_IN_PWM, 205);
      break;
    case '3':
      Serial.print("FAN INPUT -> ON at");
      mcp0.digitalWrite(FAN_IN, LOW);
      Serial.println(" 40 % PWM");
      analogWrite(FAN_IN_PWM, 410);
      break;
    case '4':
      Serial.print("FAN INPUT -> ON at");
      mcp0.digitalWrite(FAN_IN, LOW);
      Serial.println(" 70 % PWM");
      analogWrite(FAN_IN_PWM, 714);
      break;
    case '5': 
      Serial.print("FAN INPUT -> ON at");
      rdebugD("FAN INPUT -> ON at");
      mcp0.digitalWrite(FAN_IN, LOW);
      Serial.println(" 100 % PWM");
      rdebugDln(" 100 % PWM");
      // analogWrite(FAN_IN_PWM, 1024);
      break;
    default:
      Serial.println("FAN INPUT -> OFF");
      rdebugDln("FAN INPUT -> OFF");
      mcp0.digitalWrite(FAN_IN, HIGH); // Relay-off
      break;
   }
}

float getDistance() {
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
  Serial.print(timestamp + ": Recording distance...");
  rdebugD("&s: Distance: ", timestamp.c_str());
  rdebugDln("%d", distance);
  rdebugD("%d: Recording distance...", timestamp.c_str());
  char result[8];
  char* message = dtostrf(distance, 6, 2, result);
  int length = strlen(message);
  boolean retained = true;
  clientMQTT.publish("distance_result", (byte*)message, length, retained);
  Serial.println("done");
  rdebugDln("done");
  return distance;
}

float getWaterLevel() {
  float water_level = adc.readADC(0);
  Serial.println((String)timestamp + ": MCP3008_ADC_0: " + water_level); // water level
  Serial.print(timestamp + ": Recording data of water level...");
  rdebugDln("%d: MCP3008_ADC_0: %d", timestamp.c_str(), water_level); // water level
  rdebugD("%d: Recording data of water level...", timestamp.c_str());
  char result[8];
  char* message = dtostrf(water_level, 6, 2, result);
  int length = strlen(message);
  boolean retained = true;
  clientMQTT.publish("water_level_result", (byte*)message, length, retained);
  Serial.println("done");
  rdebugDln("done");
  return adc.readADC(0);
  return water_level;
}

float getTemperature() {
 // TEMPERATURE
 Serial.print((String)timestamp + "Requesting temperatures...");
 rdebugD("%d : Requesting temperatures...", timestamp.c_str());
 sensors.requestTemperatures(); // Send the command to get temperatures
 Serial.println("DONE");
 rdebugDln("DONE");
 Serial.print((String)timestamp + "Temperature for the device 1 (index 0) is: ");
 rdebugD("%d : Temperature for the device 1 (index 0) is: ", timestamp.c_str());
 float temperature = sensors.getTempCByIndex(0);
 Serial.println(temperature);
 rdebugDln("%d", temperature);
 char result[8];
 char* message = dtostrf(temperature, 6, 2, result);
 int length = strlen(message);
 boolean retained = true;
 clientMQTT.publish("temperature_result", (byte*)message, length, retained);
 return temperature;
}

float getLightSpectrum() {
  float light_spectrum = adc.readADC(2);
  Serial.println((String)timestamp + ": MCP3008_ADC_2: " + light_spectrum); // light spectrum
  Serial.println(timestamp + ": Recording data of light spectrum.");
  rdebugDln("%d: MCP3008_ADC_2: %d", timestamp.c_str(), light_spectrum); // light spectrum
  rdebugDln("%d: Recording data of light spectrum.", timestamp.c_str());
  char result[8];
  char* message = dtostrf(light_spectrum, 6, 2, result);
  int length = strlen(message);
  boolean retained = true;
  clientMQTT.publish("light_spectrum_result", (byte*)message, length, retained);
  Serial.println("done");
  rdebugDln("done");
  return light_spectrum;
}

int getMoisture() {
  int moisture = adc.readADC(1);
  Serial.println((String)timestamp + ": MCP3008_ADC_1: " + moisture); // moisture sensor
  Serial.println(timestamp + ": Recording data of moisture.");
  rdebugDln("%d: MCP3008_ADC_1: %d", timestamp.c_str(), moisture); // moisture sensor
  rdebugDln("%d: Recording data of moisture.", timestamp.c_str());
  char result[8];
  char* message = dtostrf(moisture, 6, 2, result);
  int length = strlen(message);
  boolean retained = true;
  clientMQTT.publish("moisture_result", (byte*)message, length, retained);
  Serial.println("done");
  rdebugDln("done");
  return moisture;
}

String getEnergy() {
  float v = pzem.voltage(ip);
  if (v < 0.0) v = 0.0;
  Serial.print(v);Serial.print("V; ");

  float i = pzem.current(ip);
  if(i >= 0.0){ Serial.print(i);Serial.print("A; "); }
  
  float p = pzem.power(ip);
  if(p >= 0.0){ Serial.print(p);Serial.print("W; "); }
  
  float e = pzem.energy(ip);
  if(e >= 0.0){ Serial.print(e);Serial.print("Wh; "); }

  Serial.println();
  
  char result[8];
  char* voltage_res = dtostrf(v, 6, 2, result);
  char* current_res = dtostrf(i, 6, 2, result);
  char* watt_res = dtostrf(p, 6, 2, result);
  char* count_watt_res = dtostrf(e, 6, 2, result);
  String message = String(voltage_res) + "|" + String(current_res) + "|" + String(watt_res) + "|" + String(count_watt_res);
  char charBuf[50];
  int length = message.length();
  message.toCharArray(charBuf, 50);
  boolean retained = true;
  clientMQTT.publish("energy_result", (byte*)message.c_str(), length, retained);
  
  return String(message);
}

void loop() {
  // MQTT client
  if (!clientMQTT.connected()) {
    rdebugDln("Reconnect to MQTT");
    reconnect();
  }
  clientMQTT.loop();
  
  // REMOTE UPDATE OTA
  httpServer.handleClient();

  // TELNET REMOTE DEBUG
  if (RemoteSerial) Debug.handle();
 
    // TIME TIMESTAMP
  HTTPClient clientHTTP;
  clientHTTP.begin("http://weinzuhause.altervista.org/ws/getDateTime.php");
  int httpCode = clientHTTP.GET();
  if (httpCode > 0) {
    timestamp = clientHTTP.getString();
  } else {
    Serial.printf("[HTTP] GET... failed, error: %d\n", clientHTTP.errorToString(httpCode).c_str());
    rdebugEln("[HTTP] GET... failed, error: %d\n", clientHTTP.errorToString(httpCode).c_str());
  }
  clientHTTP.end();
}

void reconnect() {
  // Loop until we're reconnected
  while (!clientMQTT.connected()) {
    Serial.print("Attempting MQTT connection...");
    rdebugI("Attempting MQTT connection...");
    // Attempt to connect
    if (clientMQTT.connect("ESP8266Client", mqttUser, mqttPassword)) {
      Serial.println("connected");
      rdebugI("connected");
      clientMQTT.subscribe("water");
      clientMQTT.subscribe("light");
      clientMQTT.subscribe("phase");
      clientMQTT.subscribe("distance");
      clientMQTT.subscribe("water_level");
      clientMQTT.subscribe("light_spectrum");
      clientMQTT.subscribe("moisture");
      clientMQTT.subscribe("fanIn");
      clientMQTT.subscribe("fanOut");
      clientMQTT.subscribe("coolLamp");
      clientMQTT.subscribe("co2");
      clientMQTT.subscribe("heater");
      clientMQTT.subscribe("peltier");
      clientMQTT.subscribe("water_pump");
      clientMQTT.subscribe("dehumidifier");
      clientMQTT.subscribe("energy");
      clientMQTT.subscribe("test");
      clientMQTT.subscribe("testSingle");
      clientMQTT.subscribe("testSingleOff");
      clientMQTT.subscribe("temperature");
      clientMQTT.subscribe("built-in_led");
    } else {
      Serial.print("failed, rc=");
      Serial.print(clientMQTT.state());
      Serial.println(" try again in 5 seconds");
      rdebugE("failed, rc=");
      rdebugE("" + clientMQTT.state());
      rdebugEln(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
