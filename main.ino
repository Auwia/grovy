/* * Author: Massimo Manganiello * */
/* 19/09/2019: New range sensore module, 3 digital meters. * version: 1.1 */
/* 04/01/2020: 3 digital meters + DS18B20 temperature sensors + fan out ON/OFF. * version: 1.2 */
/* 12/01/2020: custom delay to avoid any reset. * version: 1.2 */
/* ->    With this function, every 100 ms, the ESP wdt is fed and the cpu is yielded for any pending tasks. */
/* 13/01/2020: Save crash. * version: 1.2 */
/* 23/01/2020: MAX(temperature) and AVG(temperature), clean-up log * version: 1.2 */
/* 04/02/2020: DS18B20: clean-up log * version: 1.2 */

#include <ESP8266WiFi.h>              // WIFI
#include <PubSubClient.h>             // MQTT
#include <RemoteDebug.h>              // TELNET REMOTE DEBUG 
#include <ESP8266mDNS.h>              // TELNET REMOTE DEBUG 
#include <hcsr04.h>                   // RANGE SENSOR
#include <ESP8266WebServer.h>         // REMOTE UPDATE OTA
#include <ESP8266HTTPUpdateServer.h>  // REMOTE UPDATE OTA
#include <OneWire.h>                  // TEMPERATURE SENSORS 1-WIRE DS18B20
#include <DallasTemperature.h>        // TEMPERATURE SENSORS 1-WIRE DS18B20
#include <Esp.h>                      // SAVE CRASH

#define ECHO_PINa 16  // D0
#define TRIG_PINa  5  // D1
#define ECHO_PINb  4  // D2
#define TRIG_PINb  0  // D3
#define ECHO_PINc 12  // D6
#define TRIG_PINc 13  // D7


// WIFI
const char* ssid = "UPCA9E82C2";
const char* password_wifi =  "tp3Ya2mkhztk";
WiFiClient clientWIFI;

// MQTT client
const char* mqttServer = "192.168.0.159";
const int mqttPort = 1883;
const char* mqttUser = "";
const char* mqttPassword = "";
PubSubClient clientMQTT(clientWIFI);

// TELNET REMOTE DEBUG
bool RemoteSerial = true; //true = Remote and local serial, false = local serial only
RemoteDebug Debug;
#define MAX_SRV_CLIENTS 1
WiFiClient serverClients[MAX_SRV_CLIENTS];
const char* host = "esp8266-range_sensor";

// RANGE SENSOR
HCSR04 hcsr04a(TRIG_PINa, ECHO_PINa, 20, 4000);
HCSR04 hcsr04b(TRIG_PINb, ECHO_PINb, 20, 4000);
HCSR04 hcsr04c(TRIG_PINc, ECHO_PINc, 20, 4000);

// REMOTE UPDATE OTA
ESP8266WebServer httpServer(81);
ESP8266HTTPUpdateServer httpUpdater;

// TEMPERATURE SENSORS 1-WIRE DS18B20
#define ONE_WIRE_BUS 14
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

void setup() {
  Serial.begin(115200);

  // WIFI
  WiFi.begin(ssid, password_wifi);
  Serial.print("Connecting to WiFi.");
  while (WiFi.status() != WL_CONNECTED) {
    myDelay(500);
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
  MDNS.addService("http", "tcp", 81);
  Serial.printf("HTTPUpdateServer ready! Open http://%d.local/update in your browser\n", host);

  // TELNET REMOTE DEBUG
  MDNS.addService("telnet", "tcp", 23);
  Debug.begin(host);
  Debug.setSerialEnabled(true);
  Debug.setResetCmdEnabled(true); // Enable the reset command
  rdebugIln("TELNET REMOTE DEBUG...ok!");

  // TEMPERATURE SENSORS 1-WIRE DS18B20
  DS18B20.begin();

}

void loop() {
  // MQTT client
  if (!clientMQTT.connected()) {
    rdebugDln("Reconnect to MQTT");
    reconnect();
  }
  clientMQTT.loop();

  // TELNET REMOTE DEBUG
  if (RemoteSerial) Debug.handle();

  // REMOTE UPDATE OTA
  httpServer.handleClient();
}

void reconnect() {
  // Loop until we're reconnected
  while (!clientMQTT.connected()) {
    Serial.print("Attempting MQTT connection...");
    rdebugI("Attempting MQTT connection...");
    // Attempt to connect
    if (clientMQTT.connect("ESP8266Client-range_sensor", mqttUser, mqttPassword)) {
      Serial.println("connected");
      rdebugI("connected");
      clientMQTT.subscribe("distance_1");
      clientMQTT.subscribe("distance_2");
      clientMQTT.subscribe("distance_3");
      clientMQTT.subscribe("distance_1_result");
      clientMQTT.subscribe("distance_2_result");
      clientMQTT.subscribe("distance_3_result");
      clientMQTT.subscribe("temperature");        // TEMPERATURE SENSORS 1-WIRE DS18B20
      clientMQTT.subscribe("error");              // SAVE CRASH
    } else {
      Serial.print("failed, rc=");
      Serial.print(clientMQTT.state());
      Serial.println(" try again in 1 seconds");
      rdebugE("failed, rc=");
      rdebugE("" + clientMQTT.state());
      rdebugEln(" try again in 1 seconds");
      // Wait 5->1 seconds before retrying //BUG_001
      // delay(5000);
      myDelay(250);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  myDelay(500);

  if (String(topic).equals("distance_1")) {
    getDistance1();
  }
  if (String(topic).equals("distance_2")) {
    getDistance2();
  }
  if (String(topic).equals("distance_3")) {
    getDistance3();
  }
  // TEMPERATURE SENSORS 1-WIRE DS18B20
  if (String(topic).equals("temperature")) {
    getTemperature();
  }
  // SAVE CRASH
  if (String(topic).equals("error")) {
    getError();
  }
}

void getError() {
  rst_info* rinfo = ESP.getResetInfoPtr();
  Serial.println(rinfo->reason);
  rdebugDln("%s", ESP.getResetInfo().c_str());
  String message = ESP.getResetInfo();
  char charBuf[50];
  int length = message.length();
  message.toCharArray(charBuf, 50);
  boolean retained = true;
  clientMQTT.publish("error_result", (byte*)message.c_str(), length, retained);

}
void getDistance1() {
  Serial.print("A: ");
  rdebugD("A: ");
  float distance = hcsr04a.distanceInMillimeters() / 10;
  Serial.println(distance);
  rdebugDln("%f", distance);
  char result[8];
  char* message = dtostrf(distance, 6, 1, result);
  int length = strlen(message);
  boolean retained = true;
  clientMQTT.publish("distance_1_result", (byte*)message, length, retained);
}
void getDistance2() {
  Serial.print("B: ");
  rdebugD("B: ");
  float distance = hcsr04b.distanceInMillimeters() / 10;
  Serial.println(distance);
  rdebugDln("%f", distance);
  char result[8];
  char* message = dtostrf(distance, 6, 1, result);
  int length = strlen(message);
  boolean retained = true;
  clientMQTT.publish("distance_2_result", (byte*)message, length, retained);
}
void getDistance3() {
  Serial.print("C: ");
  rdebugD("C: ");
  float distance = hcsr04c.distanceInMillimeters() / 10;
  Serial.println(distance);
  rdebugDln("%f", distance);
  char result[8];
  char* message = dtostrf(distance, 6, 1, result);
  int length = strlen(message);
  boolean retained = true;
  clientMQTT.publish("distance_3_result", (byte*)message, length, retained);
}
// TEMPERATURE SENSORS 1-WIRE DS18B20
void getTemperature() {
  int numberOfDevices = DS18B20.getDeviceCount();
  rdebugIln("Number of device %d founds.", numberOfDevices);
  DS18B20.requestTemperatures();

  float temperature_max = 0;
  float temperature_sum = 0;
  char charBuf[50];
  int length = 0;
  String message;
  boolean retained = true;
  
  for (int i = 0; i < numberOfDevices; i++) {
    float temperature = DS18B20.getTempCByIndex(i);
    rdebugDln("Sensor nr: %d temp: %f°C", i, temperature);
    if (temperature > temperature_max) {
        temperature_max = temperature;
    }
    temperature_sum = temperature_sum + temperature;
    message = "Box_" + String(i) + "|" + String(temperature);
    length = message.length();
    message.toCharArray(charBuf, 50);  
    clientMQTT.publish("temperature_result", (byte*)message.c_str(), length, retained);
  }
  message = String(temperature_max);
  rdebugDln("MAX(temperature): %f", temperature_max);
  length = message.length();
  message.toCharArray(charBuf, 50);
  clientMQTT.publish("temperature_max_result", (byte*)message.c_str(), length, retained);

  float temperature_avg = temperature_sum / numberOfDevices;
  message = String(temperature_avg);
  rdebugDln("AVG(temperature): %f", temperature_avg);
  length = message.length();
  message.toCharArray(charBuf, 50);
  clientMQTT.publish("temperature_avg_result", (byte*)message.c_str(), length, retained);
}

void myDelay(int ms) {
  int i;
  for (i = 1; i != ms; i++) {
    delay(1);
    if (i % 100 == 0) {
      ESP.wdtFeed();
      yield();
    }
  }
}
