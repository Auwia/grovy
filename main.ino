/* * Author: Massimo Manganiello * */
/* 16/05/2018: first development. * version: 1.0 */
/* 17/06/2018: new core / feature. * version: 2.0 */
/* 21/06/2018: no mysql connection. * version: 3.0 */
/* 28/07/2018: 16ch. relay board. driver MCP23017 * version: 3.1 */
/* 20/08/2018: clean up * version: 3.2 */

#include <OneWire.h>
#include <Adafruit_MCP3008.h>
#include <Adafruit_MCP23017.h>
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <PubSubClient.h>
#include <PZEM004T.h>

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

// MQTT client
const char* mqttServer = "192.168.0.178";
const int mqttPort = 1883;
const char* mqttUser = "";
const char* mqttPassword = "";

// MCP3008
Adafruit_MCP3008 adc;
#define CLOCK_PIN D5
#define MISO_PIN D6
#define MOSI_PIN D7
#define CS_PIN D8

// RANGE SENSOR
const int trigPin = 0; //D4
const int echoPin = 2; //D3
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
const int HEATER FAN = 14;
const int HEATER = 15;

String timestamp;
 
WiFiClient espClient;
PubSubClient client(espClient);
 
void setup() { 
  Serial.begin(115200); 
  
  // PZEM004T
  pzem.setAddress(ip);
  
  WiFi.begin(ssid, password_wifi); 
  Serial.print("Connecting to WiFi.");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("ok.");
 
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);

  Serial.print("Connecting to MQTT.");
  while (!client.connected()) {
    Serial.print(".");
    if (client.connect("ESP8266Client", mqttUser, mqttPassword )) {
      Serial.println("ok.");  
    } else { 
      Serial.println("failed with state: ");
      Serial.print(client.state());
      delay(2000); 
    }
  }
 
  client.subscribe("water");
  client.subscribe("light");
  client.subscribe("phase");
  client.subscribe("distance");
  client.subscribe("water_level");
  client.subscribe("light_spectrum");
  client.subscribe("moisture");
  client.subscribe("fanIn");
  client.subscribe("fanOut");
  client.subscribe("coolLamp");
  client.subscribe("co2");
  client.subscribe("heater");
  client.subscribe("peltier");
  client.subscribe("water_pump");
  client.subscribe("dehumidifier");
  client.subscribe("energy");
  client.subscribe("test");
  client.subscribe("testSingle");
  client.subscribe("testSingleOff");
  client.subscribe("temperature");

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
 
void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    message += (char)payload[i];
  }
  Serial.println("");
  delay(1000);

  if (String(topic).equals("dehumidifier")) {
    if(message.equals("1")) {
      Serial.println("DEHUMIDIFIER->ON");
      mcp0.digitalWrite(DEHUMIDIFIER, LOW);
    } else {
      Serial.println("DEHUMIDIFIER->OFF");
      mcp0.digitalWrite(DEHUMIDIFIER, HIGH);      
    }
  }
  
  if (String(topic).equals("peltier")) {
    if(message.equals("1")) {
      Serial.println("PELTIER->ON");
      mcp0.digitalWrite(PELTIER, LOW);
    } else {
      Serial.println("PELTIER->OFF");
      mcp0.digitalWrite(PELTIER, HIGH);      
    }
  }
  
  if (String(topic).equals("heater")) {
    if(message.equals("1")) {
      Serial.println("HEATER->ON");
      mcp0.digitalWrite(HEATER, LOW);
    } else {
      Serial.println("HEATER->OFF");
      mcp0.digitalWrite(HEATER, HIGH);      
    }
  }

  if (String(topic).equals("co2")) {
    if(message.equals("1")) {
      Serial.println("CO2->ON");
      mcp0.digitalWrite(CO2, LOW);
    } else {
      Serial.println("CO2->OFF");
      mcp0.digitalWrite(CO2, HIGH);      
    }
  }
  
  if (String(topic).equals("coolLamp")) {
    if(message.equals("1")) {
      Serial.println("COOL LAMP->ON");
      mcp0.digitalWrite(COOL_LAMP, LOW);
    } else {
      Serial.println("COOL_LAMP->OFF");
      mcp0.digitalWrite(COOL_LAMP, HIGH);      
    }
  }

  if (String(topic).equals("fanOut")) {
    if(message.equals("1")) {
      Serial.println("FAN OUT->ON");
      mcp0.digitalWrite(FAN_OUT, LOW);
    } else {
      Serial.println("FAN OUT->OFF");
      mcp0.digitalWrite(FAN_OUT, HIGH);      
    }
  }
 
  if (String(topic).equals("fanIn")) {
    setFanIn((char)payload[0]);
  }
  
  if (String(topic).equals("water_pump")) {
    if(message.equals("1")) {
      Serial.println("WATER->ON");
      mcp0.digitalWrite(WATER_PUMP, LOW);
    } else {
      Serial.println("WATER->OFF");
      mcp0.digitalWrite(WATER_PUMP, HIGH);      
    }
  }

  if (String(topic).equals("testSingle")) {
    Serial.println("TEST PIN: " + message + " ->ON");
    mcp0.digitalWrite(message.toInt(), LOW);
  }
  
  if (String(topic).equals("testSingleOff")) {
    Serial.println("TEST PIN: " + message + " ->OFF");
    mcp0.digitalWrite(message.toInt(), HIGH);
  }

  if (String(topic).equals("test")) {
    if(message.equals("1")) {
      Serial.println("TEST->ON");
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
      mcp0.digitalWrite(EMPTY_II, LOW);
      Serial.println("TEST->OFF");
    } else {
      Serial.println("TEST->ON");
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
      mcp0.digitalWrite(EMPTY_II, HIGH);
      Serial.println("TEST->OFF");      
    }     
  }
     
  if (String(topic).equals("phase")) {
    switch ((char)payload[0]) {
      // PHASE : (0: GERMINATION, 1: VEGETABLE_LOW, 2: VEGETABLE_HIGH, 3: FLOWERING_LOW, 4: FLOWERING_HIGH)
      case '0':
        Serial.println("PHASE=GERMINATION");
        PHASE = "GERMINATION";
        break;
      case '1':
        Serial.println("PHASE=VEGETABLE_LOW");
        PHASE = "VEGETABLE_LOW";
        break;
      case '2':
        Serial.println("PHASE=VEGETABLE_HIGH");
        PHASE = "VEGETABLE_HIGH";
        break;
      case '3':
        Serial.println("PHASE=FLOWERING_LOW");
        PHASE = "FLOWERING_LOW";
        break;
      case '4':
        Serial.println("PHASE=FLOWERING_HIGH");
        PHASE = "FLOWERING_HIGH";
        break;
    }
  }
  if (String(topic).equals("distance")) {
    Serial.print("distance: ");
    float distance = getDistance();
    Serial.println(distance);
  }
  if (String(topic).equals("water_level")) {
    Serial.print("water_level: ");
    float water_level = getWaterLevel();
    Serial.println(water_level);
  }
  if (String(topic).equals("light_spectrum")) {
    Serial.print("light_spectrum: ");
    int light_spectrum = getLightSpectrum();
    Serial.println(light_spectrum);
  }
  if (String(topic).equals("temperature")) {
    Serial.print("temperature: ");
    float temperature = getTemperature();
    Serial.println(temperature);
  }
  if (String(topic).equals("moisture")) {
    Serial.print("moisture: ");
    int moisture = getMoisture();
    Serial.println(moisture);
  }
  if (String(topic).equals("energy")) {
    Serial.print("Energy: ");
    String energy_res = getEnergy();
    Serial.println(energy_res);
  }
  Serial.println();
  Serial.println("-----------------------");
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
      mcp0.digitalWrite(FAN_IN, LOW);
      Serial.println(" 100 % PWM");
      // analogWrite(FAN_IN_PWM, 1024);
      break;
    default:
      Serial.println("FAN INPUT -> OFF");
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
  char result[8];
  char* message = dtostrf(distance, 6, 2, result);
  int length = strlen(message);
  boolean retained = true;
  client.publish("distance_result", (byte*)message, length, retained);
  Serial.println("done");
  return distance;
}

float getWaterLevel() {
  float water_level = adc.readADC(0);
  Serial.println((String)timestamp + ": MCP3008_ADC_0: " + water_level); // water level
  Serial.print(timestamp + ": Recording data of water level...");
  char result[8];
  char* message = dtostrf(water_level, 6, 2, result);
  int length = strlen(message);
  boolean retained = true;
  client.publish("water_level_result", (byte*)message, length, retained);
  Serial.println("done");
  return adc.readADC(0);
  return water_level;
}

float getTemperature() {
 // TEMPERATURE
 Serial.print((String)timestamp + "Requesting temperatures...");
 sensors.requestTemperatures(); // Send the command to get temperatures
 Serial.println("DONE");
 Serial.print((String)timestamp + "Temperature for the device 1 (index 0) is: ");
 float temperature = sensors.getTempCByIndex(0);
 Serial.println(temperature);
 char result[8];
 char* message = dtostrf(temperature, 6, 2, result);
 int length = strlen(message);
 boolean retained = true;
 client.publish("temperature_result", (byte*)message, length, retained);
 return temperature;
}

float getLightSpectrum() {
  float light_spectrum = adc.readADC(2);
  Serial.println((String)timestamp + ": MCP3008_ADC_2: " + light_spectrum); // light spectrum
  Serial.println(timestamp + ": Recording data of light spectrum.");
  char result[8];
  char* message = dtostrf(light_spectrum, 6, 2, result);
  int length = strlen(message);
  boolean retained = true;
  client.publish("light_spectrum_result", (byte*)message, length, retained);
  Serial.println("done");
  return light_spectrum;
}

int getMoisture() {
  int moisture = adc.readADC(1);
  Serial.println((String)timestamp + ": MCP3008_ADC_1: " + moisture); // moisture sensor
  Serial.println(timestamp + ": Recording data of moisture.");
  char result[8];
  char* message = dtostrf(moisture, 6, 2, result);
  int length = strlen(message);
  boolean retained = true;
  client.publish("moisture_result", (byte*)message, length, retained);
  Serial.println("done");
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
  client.publish("energy_result", (byte*)message.c_str(), length, retained);
  
  return String(message);
}

void loop() {
  client.loop();
    // TIME TIMESTAMP
  HTTPClient http;
  http.begin("http://weinzuhause.altervista.org/ws/getDateTime.php");
  int httpCode = http.GET();
  if (httpCode > 0) {
    timestamp = http.getString();
  } else {
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();

  if (PHASE == "GERMINATION") {
    mcp0.digitalWrite(LED_WARM_WHITE, LOW);
    mcp0.digitalWrite(LED_FULL_SPECTRUM_UV, LOW);
    mcp0.digitalWrite(LED_ROYAL_BLUE, LOW);
    mcp0.digitalWrite(LED_BRIGHT_BLUE, HIGH);
    mcp0.digitalWrite(LED_BRIGHT_RED, HIGH);
    mcp0.digitalWrite(LED_DEEP_RED, HIGH);
  }
  if (PHASE == "VEGETABLE_LOW") {
    mcp0.digitalWrite(LED_WARM_WHITE, HIGH);
    mcp0.digitalWrite(LED_FULL_SPECTRUM_UV, LOW);
    mcp0.digitalWrite(LED_ROYAL_BLUE, LOW);
    mcp0.digitalWrite(LED_BRIGHT_BLUE, LOW);
    mcp0.digitalWrite(LED_BRIGHT_RED, HIGH);
    mcp0.digitalWrite(LED_DEEP_RED, HIGH);
  }
  if (PHASE == "VEGETABLE_HIGH") {
    mcp0.digitalWrite(LED_WARM_WHITE, HIGH);
    mcp0.digitalWrite(LED_FULL_SPECTRUM_UV, LOW);
    mcp0.digitalWrite(LED_ROYAL_BLUE, LOW);
    mcp0.digitalWrite(LED_BRIGHT_BLUE, LOW);
    mcp0.digitalWrite(LED_BRIGHT_RED, HIGH);
    mcp0.digitalWrite(LED_DEEP_RED, HIGH);
  }
  if (PHASE == "FLOWERING_LOW") {
    mcp0.digitalWrite(LED_WARM_WHITE, HIGH);
    mcp0.digitalWrite(LED_FULL_SPECTRUM_UV, LOW);
    mcp0.digitalWrite(LED_ROYAL_BLUE, HIGH);
    mcp0.digitalWrite(LED_BRIGHT_BLUE, HIGH);
    mcp0.digitalWrite(LED_BRIGHT_RED, LOW);
    mcp0.digitalWrite(LED_DEEP_RED, LOW);
  }
  if (PHASE == "FLOWERING_HIGH") {
    mcp0.digitalWrite(LED_WARM_WHITE, HIGH);
    mcp0.digitalWrite(LED_FULL_SPECTRUM_UV, LOW);
    mcp0.digitalWrite(LED_ROYAL_BLUE, HIGH);
    mcp0.digitalWrite(LED_BRIGHT_BLUE, HIGH);
    mcp0.digitalWrite(LED_BRIGHT_RED, HIGH);
    mcp0.digitalWrite(LED_DEEP_RED, LOW);
  }
}
