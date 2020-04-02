/* * Author: Massimo Manganiello * first development: 31/03/2020 * version: 1.0 */
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <Adafruit_MCP3008.h>
#include <ESP8266WiFi.h>              // WIFI
#include <ESP8266WebServer.h>         // REMOTE UPDATE OTA
#include <ESP8266HTTPUpdateServer.h>  // REMOTE UPDATE OTA
#include <ESP8266mDNS.h>              // TELNET REMOTE DEBUG
#include <RemoteDebug.h>              // TELNET REMOTE DEBUG 
#include <PubSubClient.h>             // MQTT 
#include <ESP8266HTTPClient.h>

// WIFI
const char* ssid = "UPCA9E82C2";
const char* password_wifi =  "tp3Ya2mkhztk";
WiFiClient clientWIFI;

// MQTT client
const char* mqttServer = "192.168.0.178";
const int mqttPort = 1883;
const char* mqttUser = "";
const char* mqttPassword = "";
PubSubClient clientMQTT(clientWIFI);

// TELNET REMOTE DEBUG
bool RemoteSerial = true; //true = Remote and local serial, false = local serial only
RemoteDebug Debug;
#define MAX_SRV_CLIENTS 1
WiFiClient serverClients[MAX_SRV_CLIENTS];
const char* host = "esp8266-lower";

// REMOTE UPDATE OTA
ESP8266WebServer httpServer(82);
ESP8266HTTPUpdateServer httpUpdater;

#define DHTPIN 0     // Digital pin connected to the DHT sensor

// Uncomment the type of sensor in use:
#define DHTTYPE    DHT11     // DHT 11
//#define DHTTYPE    DHT22     // DHT 22 (AM2302)
//#define DHTTYPE    DHT21     // DHT 21 (AM2301)
DHT dht(DHTPIN, DHTTYPE);
// current temperature & humidity, updated in loop()
float t = 0.0;
float h = 0.0;

Adafruit_MCP3008 adc;
int count = 0;


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
  MDNS.addService("http", "tcp", 82);
  Serial.printf("HTTPUpdateServer ready! Open http://%d.local/update in your browser\n", host);

  // TELNET REMOTE DEBUG
  MDNS.addService("telnet", "tcp", 23);
  Debug.begin(host);
  Debug.setSerialEnabled(true);
  Debug.setResetCmdEnabled(true); // Enable the reset command
  rdebugIln("TELNET REMOTE DEBUG...ok!");

  pinMode(5, OUTPUT);
  pinMode(4, OUTPUT);
  digitalWrite(5, LOW);
  digitalWrite(4, HIGH);
  dht.begin();
  adc.begin();
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

  myDelay(250);
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

void reconnect() {
  // Loop until we're reconnected
  while (!clientMQTT.connected()) {
    Serial.print("Attempting MQTT connection...");
    rdebugI("Attempting MQTT connection...");
    // Attempt to connect
    if (clientMQTT.connect("esp8266-lower", mqttUser, mqttPassword)) {
      Serial.println("connected");
      rdebugI("connected");
      clientMQTT.subscribe("moisture");           // MOISTURE
      clientMQTT.subscribe("temp_humy_out");  // TEMPERATURE SENSORS HTC-11
      clientMQTT.subscribe("water_pump");         // WATER PUMP
      clientMQTT.subscribe("fanIn");              // FAN IN
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

  if (String(topic).equals("moisture")) {
    Serial.print("moisture: " + String(topic) + ": ");
    rdebugD("moisture: ");
    int moisture = 0;

    if (message.equals("0")) {
      moisture = getMoisture(0);
    }
    if (message.equals("1")) {
      moisture = getMoisture(1);
    }
    if (message.equals("2")) {
      moisture = getMoisture(2);
    }

    Serial.println(moisture);
    rdebugDln("result: moisture: %d", moisture);
  }

  // TEMPERATURE HTC11
  if (String(topic).equals("temp_humy_out")) {
    getTemperature();
  }

  // WATER PUMP
  if (String(topic).equals("water_pump")) {
    setPump(message);
  }

  if (String(topic).equals("fanIn")) {
    setFanIn(message);
  }

  myDelay(500);
}

void setPump(String message) {
  if (message.equals("0")) {
    Serial.println("Relay 1 : 4 - High.");
    rdebugDln("Relay 1 : 4 - High.");
    digitalWrite(4, HIGH);
  } else {
    Serial.println("Relay 1 : 4 - Low.");
    rdebugDln("Relay 1 : 4 - Low.");
    digitalWrite(4, LOW);
  }
}

void setFanIn(String message) {
  if (message.equals("0")) {
    Serial.println("Relay 2 : 5 - High.");
    rdebugDln("Relay 2 : 5 - High.");
    digitalWrite(5, HIGH);
  } else {
    Serial.println("Relay 2 : 5 - Low.");
    rdebugDln("Relay 2 : 5 - Low.");
    digitalWrite(5, LOW);
  }
}

void getTemperature() {
  float newT = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  //float newT = dht.readTemperature(true);
  // if temperature read failed, don't change t value
  if (isnan(newT)) {
    Serial.println("Failed to read from DHT sensor!");
    rdebugDln("Failed to read from DHT sensor!");
  }
  else {
    t = newT;
    Serial.print("Temperature: ");
    Serial.println(t);
    rdebugDln("Temperature: %f", t);
  }
  // Read Humidity
  float newH = dht.readHumidity();
  // if humidity read failed, don't change h value
  if (isnan(newH)) {
    Serial.println("Failed to read from DHT sensor!");
    rdebugDln("Failed to read from DHT sensor!");
  } else {
    h = newH;
    Serial.print("Humidity: ");
    Serial.println(h);
    rdebugDln("Humidity: %f", h);
  }

  char result[8];
  char* temperature = dtostrf(t, 6, 1, result);
  char* humidity = dtostrf(h, 6, 1, result);
  String message = String(temperature) + "|" + String(humidity);
  char charBuf[50];
  int length = message.length();
  message.toCharArray(charBuf, 50);
  boolean retained = true;
  clientMQTT.publish("temp_humy_outside", (byte*)message.c_str(), length, retained);
}

int getMoisture(int sensor) {
  int moisture = 0;
  switch (sensor) {
    case 0:
      rdebugDln("Selected sensor: 0");
      moisture = adc.readADC(0);
      break;
    case 1:
      rdebugDln("Selected sensor: 1");
      moisture = adc.readADC(1);
      break;
    case 2:
      rdebugDln("Selected sensor: 2");
      moisture = adc.readADC(2);
      break;
    default:
      Serial.println("Invalid sensor number: " + sensor);
      rdebugDln("Invalid sensor number: %d", sensor);
      Serial.println("Default sensor 0");
      rdebugDln("Default sensor 0");
      moisture = adc.readADC(1);
      break;
  }

  Serial.println(getTimestamp() + ": Sensor n.: " + sensor + " : " + moisture);
  Serial.println(getTimestamp() + ": Recording data of moisture from sensor n.: " + sensor + ".");
  rdebugDln("%s: Sensor n. %d: %d", getTimestamp().c_str(), sensor, moisture);
  rdebugDln("%s: Recording data of moisture from sensor n. %d.", getTimestamp().c_str(), sensor);
  char result[8];
  char* message = dtostrf(moisture, 6, 2, result);
  int length = strlen(message);
  boolean retained = true;
  switch (sensor) {
    case 0:
      clientMQTT.publish("moisture_result_0", (byte*)message, length, retained);
      break;
    case 1:
      clientMQTT.publish("moisture_result_1", (byte*)message, length, retained);
      break;
    case 2:
      clientMQTT.publish("moisture_result_2", (byte*)message, length, retained);
      break;
    default:
      clientMQTT.publish("moisture_result_0", (byte*)message, length, retained);
      break;
  }
  Serial.println("done");
  rdebugDln("done");
  return moisture;
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
