/* * Author: Massimo Manganiello * */
/* 23-25/03/2019: Lamp first development. * version: 1.0 */

#include <Adafruit_MCP3008.h>
#include <Adafruit_MCP23017.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <PZEM004T.h>
#include <RemoteDebug.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>

// TIMESTAMP
String timestamp;

// TEMPERATURE
#define ONE_WIRE_BUS 12
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);


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

// LEDs
int COOL_LAMP        =  4;
int OUT_FILTER       =  2;
int FULL_SPECTRUM    = 13;
int BRIGHT_BLU       = 14;
int BRIGHT_RED_1     =  5;
int ROYAL_BLU        = 16;
int DEEP_RED         =  0;

// REMOTE UPDATE OTA
const char* host = "esp8266-webupdate";
ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

// TELNET REMOTE DEBUG
bool RemoteSerial = true; //true = Remote and local serial, false = local serial only
RemoteDebug Debug;
#define MAX_SRV_CLIENTS 1
WiFiClient serverClients[MAX_SRV_CLIENTS];

// FAN PWM
const int FAN_IN_PWM = 16;

// MYSQL
IPAddress server_addr(185, 51, 11, 3);
char user[] = "u112031db1";
char password[] = "4i340So";
MySQL_Connection conn((Client *)&clientWIFI);
MySQL_Cursor *cur_mem;

void setup() {
  Serial.begin(115200);

  // TEMPERATURE
  DS18B20.begin();

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

  // MYSQL
  if (conn.connect(server_addr, 3306, user, password)) {
    delay(1000);
  } else Serial.println("Connection failed.");

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

  if (String(topic).equals("coolLamp")) {
    pinMode(COOL_LAMP, OUTPUT);
    if (message.equals("1")) {
      Serial.println("COOL LAMP->ON");
      rdebugDln("COOL LAMP->ON");
      digitalWrite(COOL_LAMP, LOW);
    } else {
      Serial.println("COOL_LAMP->OFF");
      rdebugDln("COOL_LAMP->OFF");
      digitalWrite(COOL_LAMP, HIGH);
    }
  }

  if (String(topic).equals("out_filter")) {
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
      Serial.println("BRIGHT_RED LED->OFF");
      rdebugDln("BRIGHT_RED LED->OFF");
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

  if (String(topic).equals("full_spectrum")) {
    pinMode(FULL_SPECTRUM, OUTPUT);
    if (message.equals("1")) {
      Serial.println("FULL_SPECTRUM LED->ON");
      rdebugDln("FULL_SPECTRUM LED->ON");
      digitalWrite(FULL_SPECTRUM, LOW);
    } else {
      Serial.println("FULL_SPECTRUM LED->OFF");
      rdebugDln("FULL_SPECTRUM LED->OFF");
      digitalWrite(FULL_SPECTRUM, HIGH);
    }
  }

  if (String(topic).equals("temperature")) {
    int numberOfDevices = DS18B20.getDeviceCount();
    rdebugIln("Number of device %d founds.", numberOfDevices);
    DS18B20.requestTemperatures();

    for (int i = 0; i < numberOfDevices; i++) {
      rdebugD("Sensore ");
      rdebugD("%d", i);
      rdebugD(": ");
      rdebugD("%f", DS18B20.getTempCByIndex(i));
      rdebugD(" gradi C");
      rdebugDln();
      float temperature = DS18B20.getTempCByIndex(i);
      // String INSERT_SQL = (String)"insert into u112031db1.mis (timestamp, area, temperature) values (convert_tz(now(),@@session.time_zone,\'+02:00\'), \'Box_" + i + "\', " + temperature + ")";
      // cur_mem = new MySQL_Cursor(&conn);
      // cur_mem->execute(INSERT_SQL.c_str());
      // delete cur_mem;
    }
  }

  if (String(topic).equals("testSingle")) {
    Serial.println("TEST PIN: " + message + " ->ON");
    rdebugDln("TEST PIN: %s ->ON", message.c_str());
  }

  if (String(topic).equals("testSingleOff")) {
    Serial.println("TEST PIN: " + message + " ->OFF");
    rdebugDln("TEST PIN: %s ->OFF", message.c_str());
  }

  if (String(topic).equals("test")) {
    if (message.equals("1")) {
      Serial.println("TEST->ON");
      rdebugDln("TEST->ON");
      Serial.println("TEST->OFF");
      rdebugDln("TEST->OFF");
    } else {
      Serial.println("TEST->ON");
      rdebugDln("TEST->ON");
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
      clientMQTT.subscribe("full_spectrum");
      clientMQTT.subscribe("royal_blu");
      clientMQTT.subscribe("light");
      clientMQTT.subscribe("phase");
      clientMQTT.subscribe("coolLamp");
      clientMQTT.subscribe("out_filter");
      clientMQTT.subscribe("temperature");
      clientMQTT.subscribe("test");
      clientMQTT.subscribe("testSingle");
      clientMQTT.subscribe("testSingleOff");
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
