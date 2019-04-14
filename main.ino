#include <ESP8266WiFi.h>  // WIFI
#include <PubSubClient.h> // MQTT
#include <RemoteDebug.h>  // TELNET REMOTE DEBUG 
#include <ESP8266mDNS.h>  // TELNET REMOTE DEBUG 
#include <hcsr04.h>       // RANGE SENSOR

#define TRIG_PINa 2
#define ECHO_PINa 0
#define TRIG_PINb 14 //D5
#define ECHO_PINb 12 //D6

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
const char* host = "esp8266-range_sensor";

HCSR04 hcsr04a(TRIG_PINa, ECHO_PINa, 20, 4000);
HCSR04 hcsr04b(TRIG_PINb, ECHO_PINb, 20, 4000);

void setup(){
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

  // TELNET REMOTE DEBUG
  MDNS.addService("telnet", "tcp", 23);
  Debug.begin(host);
  Debug.setSerialEnabled(true);
  Debug.setResetCmdEnabled(true); // Enable the reset command
  rdebugIln("TELNET REMOTE DEBUG...ok!");

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
      clientMQTT.subscribe("distance_1_result");
      clientMQTT.subscribe("distance_2_result");
    } else {
      Serial.print("failed, rc=");
      Serial.print(clientMQTT.state());
      Serial.println(" try again in 1 seconds");
      rdebugE("failed, rc=");
      rdebugE("" + clientMQTT.state());
      rdebugEln(" try again in 1 seconds");
      // Wait 5->1 seconds before retrying //BUG_001
      // delay(5000);
      delay(250);
    }
  }
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

  if (String(topic).equals("distance_1")) {
    getDistance1();
  }
  if (String(topic).equals("distance_2")) {
    getDistance2();
  }
}

void getDistance1(){
  Serial.print("A: ");
  float distance = hcsr04a.distanceInMillimeters() / 10;
  Serial.println(distance);
  char result[8];
  char* message = dtostrf(distance, 6, 1, result);
  int length = strlen(message);
  boolean retained = true;
  clientMQTT.publish("distance_1_result", (byte*)message, length, retained);
}
void getDistance2(){
  Serial.print("B: ");
  float distance = hcsr04b.distanceInMillimeters() / 10;
  Serial.println(distance);
  char result[8];
  char* message = dtostrf(distance, 6, 1, result);
  int length = strlen(message);
  boolean retained = true;
  clientMQTT.publish("distance_2_result", (byte*)message, length, retained);
}
