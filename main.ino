/* * Author: Massimo Manganiello * */
/* 13/10/2020: Lamp internal. * version: 1.3 */

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <RemoteDebug.h>
#include <OneWire.h>                  // TEMPERATURE SENSORS 1-WIRE DS18B20
#include <DallasTemperature.h>        // TEMPERATURE SENSORS 1-WIRE DS18B20

// TIMESTAMP
String timestamp;

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

int ULTRA_VIOLET     =  5;
int OUT_FILTER       = 12;
int WHITE_LED        = 16;
int BRIGHT_BLU       =  2;
int BRIGHT_RED_1     =  0;
int ROYAL_BLU        =  4;
int DEEP_RED         = 14;

// REMOTE UPDATE OTA
const char* host = "esp8266-lamp";
ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

// TELNET REMOTE DEBUG
bool RemoteSerial = true; //true = Remote and local serial, false = local serial only
RemoteDebug Debug;
#define MAX_SRV_CLIENTS 1
WiFiClient serverClients[MAX_SRV_CLIENTS];

// TEMPERATURE SENSORS 1-WIRE DS18B20
#define ONE_WIRE_BUS 13
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

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

  // TEMPERATURE SENSORS 1-WIRE DS18B20
  DS18B20.begin();

  delay(500);
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
  delay(500);

  if (String(topic).equals("ultra_violet")) {
    pinMode(ULTRA_VIOLET, OUTPUT);
    if (message.equals("1")) {
      Serial.println("ULTRA_VIOLET->ON");
      rdebugDln("ULTRA_VIOLET->ON");
      digitalWrite(ULTRA_VIOLET, LOW);
    } else {
      Serial.println("ULTRA_VIOLET->OFF");
      rdebugDln("ULTRA_VIOLET->OFF");
      digitalWrite(ULTRA_VIOLET, HIGH);
    }
  }

  if (String(topic).equals("fanOut")) {
    pinMode(OUT_FILTER, OUTPUT);
    if (message.equals("1")) {
      Serial.println("OUT FILTER ->ON");
      rdebugDln("OUT FILTER->ON");
      digitalWrite(OUT_FILTER, LOW);
    } else {
      Serial.println("OUT FILTER->OFF");
      rdebugDln("OUT FILTER->OFF");
      digitalWrite(OUT_FILTER, HIGH);
    }
  }

  if (String(topic).equals("deep_red")) {
    pinMode(DEEP_RED, OUTPUT);
    if (message.equals("1")) {
      Serial.println("DEEP_RED LED->ON");
      rdebugDln("DEEP_RED LED->ON");
      digitalWrite(DEEP_RED, LOW);
    } else {
      Serial.println("DEEP_RED LED->OFF");
      rdebugDln("DEEP_RED LED->OFF");
      digitalWrite(DEEP_RED, HIGH);
    }
  }

  if (String(topic).equals("bright_red")) {
    pinMode(BRIGHT_RED_1, OUTPUT);
    if (message.equals("1")) {
      Serial.println("BRIGHT_RED_1 LED->ON");
      rdebugDln("BRIGHT_RED_1 LED->ON");
      digitalWrite(BRIGHT_RED_1, LOW);
    } else {
      Serial.println("BRIGHT_RED_1 LED->OFF");
      rdebugDln("BRIGHT_RED_1 LED->OFF");
      digitalWrite(BRIGHT_RED_1, HIGH);
    }
  }

  if (String(topic).equals("bright_blu")) {
    pinMode(BRIGHT_BLU, OUTPUT);
    if (message.equals("1")) {
      Serial.println("BRIGHT_BLU LED->ON");
      rdebugDln("BRIGHT_BLU LED->ON");
      digitalWrite(BRIGHT_BLU, LOW);
    } else {
      Serial.println("BRIGHT_BLU LED->OFF");
      rdebugDln("BRIGHT_BLU LED->OFF");
      digitalWrite(BRIGHT_BLU, HIGH);
    }
  }

  if (String(topic).equals("royal_blu")) {
    pinMode(ROYAL_BLU, OUTPUT);
    if (message.equals("1")) {
      Serial.println("ROYAL_BLU LED->ON");
      rdebugDln("ROYAL_BLU LED->ON");
      digitalWrite(ROYAL_BLU, LOW);
    } else {
      Serial.println("ROYAL_BLU LED->OFF");
      rdebugDln("ROYAL_BLU LED->OFF");
      digitalWrite(ROYAL_BLU, HIGH);
    }
  }

  if (String(topic).equals("white")) {
    pinMode(WHITE_LED, OUTPUT);
    if (message.equals("1")) {
      Serial.println("WHITE LED->ON");
      rdebugDln("WHITE LED->ON");
      digitalWrite(WHITE_LED, LOW);
    } else {
      Serial.println("WHITE LED->OFF");
      rdebugDln("WHITE LED->OFF");
      digitalWrite(WHITE_LED, HIGH);
    }
  }

  // TEMPERATURE SENSORS 1-WIRE DS18B20
  if (String(topic).equals("temperature")) {
    getTemperature();
  }
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
      clientMQTT.subscribe("deep_red");
      clientMQTT.subscribe("bright_red");
      clientMQTT.subscribe("bright_blu");
      clientMQTT.subscribe("white");
      clientMQTT.subscribe("royal_blu");
      clientMQTT.subscribe("ultra_violet");
      clientMQTT.subscribe("fanOut");
      clientMQTT.subscribe("temperature");
    } else {
      Serial.print("failed, rc=");
      Serial.print(clientMQTT.state());
      Serial.println(" try again in 1 seconds");
      rdebugE("failed, rc=");
      rdebugE("" + clientMQTT.state());
      rdebugEln(" try again in 1 seconds");
      // Wait 5->1 seconds before retrying //BUG_001
      // delay(5000);
      delay(1000);
    }
  }
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

String getTimestamp() {
  // TIME TIMESTAMP
  String timestamp = " ";
  HTTPClient clientHTTP;
  clientHTTP.begin("http://weinzuhause.altervista.org/ws/getDateTime.php");
  int httpCode = clientHTTP.GET();
  if (httpCode > 0) {
    timestamp = clientHTTP.getString();
  } else {
    Serial.printf("[HTTP] GET... failed, error: %d\n", clientHTTP.errorToString(httpCode).c_str());
    rdebugEln("[HTTP] GET... failed, error: %d\n", clientHTTP.errorToString(httpCode).c_str());
    timestamp = "ERROR";
  }
  clientHTTP.end();
  return timestamp;
}
