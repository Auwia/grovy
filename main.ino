/* * Author: Massimo Manganiello * */
/* 16/05/2018: first development. * version: 1.0 */
/* 17/06/2018: new core / feature. * version: 2.0 */
/* 21/06/2018: no mysql connection. * version: 3.0 */
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
#define ONE_WIRE_BUS 10 //Pin to which is attached a temperature sensor 
#define ONE_WIRE_MAX_DEV 1 //The maximum number of devices 
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

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
const int LED_DEEP_RED_1 = 8; // TO ACCORPATE
const int LED_DEEP_RED_2 = 9; // TO ACCORPATE
const int LED_BRIGHT_RED = 10;
const int LED_BRIGHT_BLUE_1 = 11; // TO ACCORPATE
const int LED_BRIGHT_BLUE_2 = 12; // TO ACCORPATE
const int LED_ROYAL_BLUE = 13;
const int WATER_PUMP = 14;
const int HEATER = 15;
const int PELTIER = 16;       // TO REMOVE
const int DEHUMIDIFIER = 17;  // TO REMOVE

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
  client.subscribe("fanOutLeft");
  client.subscribe("fanOutRight");
  client.subscribe("fanLed");
  client.subscribe("co2");
  client.subscribe("heater");
  client.subscribe("peltier");
  client.subscribe("water_pump");
  client.subscribe("dehumidifier");
  client.subscribe("energy");

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
      mcp0.digitalWrite(DEHUMIDIFIER, HIGH);
    } else {
      Serial.println("DEHUMIDIFIER->OFF");
      mcp0.digitalWrite(DEHUMIDIFIER, LOW);      
    }
  }
  
  if (String(topic).equals("peltier")) {
    if(message.equals("1")) {
      Serial.println("PELTIER->ON");
      mcp0.digitalWrite(PELTIER, HIGH);
    } else {
      Serial.println("PELTIER->OFF");
      mcp0.digitalWrite(PELTIER, LOW);      
    }
  }
  
  if (String(topic).equals("heater")) {
    if(message.equals("1")) {
      Serial.println("HEATER->ON");
      mcp0.digitalWrite(HEATER, HIGH);
    } else {
      Serial.println("HEATER->OFF");
      mcp0.digitalWrite(HEATER, LOW);      
    }
  }

  if (String(topic).equals("co2")) {
    if(message.equals("1")) {
      Serial.println("CO2->ON");
      mcp0.digitalWrite(CO2, HIGH);
    } else {
      Serial.println("CO2->OFF");
      mcp0.digitalWrite(CO2, LOW);      
    }
  }
  
  if (String(topic).equals("fanLed")) {
    if(message.equals("1")) {
      Serial.println("FAN LED->ON");
      mcp0.digitalWrite(FANLED, HIGH);
    } else {
      Serial.println("FAN LED->OFF");
      mcp0.digitalWrite(FANLED, LOW);      
    }
  }

  if (String(topic).equals("fanOutLeft")) {
    if(message.equals("1")) {
      Serial.println("FAN OUT LEFT->ON");
      mcp0.digitalWrite(FAN1OUT, HIGH);
    } else {
      Serial.println("FAN OUT LEFT->OFF");
      mcp0.digitalWrite(FAN1OUT, LOW);      
    }
  }

  
  if (String(topic).equals("fanOutRight")) {
    if(message.equals("1")) {
      Serial.println("FAN OUT RIGHT->ON");
      mcp0.digitalWrite(FAN2OUT, HIGH);
    } else {
      Serial.println("FAN OUT RIGHT->OFF");
      mcp0.digitalWrite(FAN2OUT, LOW);      
    }
  }
  
  if (String(topic).equals("fanIn")) {
    setFanIn((char)payload[0]);
  }
  
  if (String(topic).equals("water_pump")) {
    if(message.equals("1")) {
      Serial.println("WATER->ON");
      mcp0.digitalWrite(WATER_PUMP, HIGH);
    } else {
      Serial.println("WATER->OFF");
      mcp0.digitalWrite(WATER_PUMP, LOW);      
    }
  }
  
  if (String(topic).equals("light")) {
    if (message.equals("1")) {
      Serial.println("LIGHT->ON");
      LIGHT = "ON";
    } else {
      Serial.println("LIGHT->OFF");
      LIGHT = "OFF";}
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
      mcp0.digitalWrite(FAN1IN, HIGH);
      Serial.println("10 % PWM");   
      analogWrite(FAN2IN, 102);
      break;
    case '2':
      Serial.print("FAN INPUT -> ON at");
      mcp0.digitalWrite(FAN1IN, LOW);
      Serial.println("20 % PWM");
      analogWrite(FAN2IN, 205);
      break;
    case '3':
      Serial.print("FAN INPUT -> ON at");
      mcp0.digitalWrite(FAN1IN, LOW);
      Serial.println("40 % PWM");
      analogWrite(FAN2IN, 410);
      break;
    case '4':
      Serial.print("FAN INPUT -> ON at");
      mcp0.digitalWrite(FAN1IN, LOW);
      Serial.println("70 % PWM");
      analogWrite(FAN2IN, 714);
      break;
    case '5': 
      Serial.print("FAN INPUT -> ON at");
      mcp0.digitalWrite(FAN1IN, LOW);
      Serial.println("100 % PWM");
      // analogWrite(FAN2IN, 1024);
      break;
    default:
      Serial.println("FAN INPUT -> OFF");
      mcp0.digitalWrite(FAN1IN, HIGH); // Relay-off
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
}
