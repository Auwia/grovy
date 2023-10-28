/* * Author: Massimo Manganiello * first development: 27/10/2023 * version: 1.0 */
#include <Adafruit_MCP23X08.h>
#include <Adafruit_MCP23X17.h>
#include <Adafruit_MCP23XXX.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Ds1302.h>
#include <Wire.h>
#include <WiFiUdp.h>

const char* ssidRouter = "UPC2522560";
const char* passwordRouter = "7FxuryjpTtus";

const char* ssidAP = "Prototype_001";
const char* passwordAP = "PasswordAP";

ESP8266WebServer server(80);

// Imposta i pin per RTC DS1302
const int PIN_DAT = D7;
const int PIN_CLK = D6;
const int RST_PIN = D8;

Ds1302 rtc(RST_PIN, PIN_CLK, PIN_DAT);

bool isAPMode = true;

Adafruit_MCP23X17 mcp;

// Imposta il server NTP
unsigned long lastSyncTime = 0;
bool clockSynced = false;
Ds1302::DateTime ntpDateTime;

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;      // Fuso orario (1 ora in secondi)
const int daylightOffset_sec = 3600;  // Ora legale (1 ora in secondi)

WiFiUDP udp;

// Dichiarazione di un array per memorizzare lo stato dei relè
bool relayState[16] = { LOW };  // Inizialmente tutti i relè spenti

void setup() {
  Serial.begin(9600);

  // Imposta l'ESP8266 in modalità Router o AP in base alla variabile isAPMode
  if (isAPMode) {
    Serial.println("Sono un AP...");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssidAP, passwordAP);
  } else {
    WiFi.begin(ssidRouter, passwordRouter);
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.println("Connessione al Wi-Fi...");
    }
  }

  if (!mcp.begin_I2C()) {
    Serial.println("Error.");
    while (1)
      ;
  }

  for (int i = 0; i < 16; i++) {
    // configure pin for output
    mcp.pinMode(i, OUTPUT);
  }

  // Inizializza il modulo DS1302
  rtc.init();
  // test if clock is halted and set a date-time (see example 2) to start it
  if (rtc.isHalted()) {
    Serial.println("RTC is halted. Setting time...");
    Ds1302::DateTime dt = {
      .year = 2023,
      .month = Ds1302::MONTH_OCT,
      .day = 28,
      .hour = 16,
      .minute = 14,
      .second = 30,
      .dow = Ds1302::DOW_TUE
    };

    rtc.setDateTime(&dt);
  }

  server.on("/", HTTP_GET, []() {
    String paginaHTML = "<html><body>";
    paginaHTML += "<h1>Controllo dei Relè</h1>";

    paginaHTML += "<form action='/setrelays' method='post'>";  // Apri il form per la richiesta POST

    for (int i = 0; i < 16; i++) {
      paginaHTML += "Relè " + String(i) + ": ";
      paginaHTML += "<input type='checkbox' name='rele" + String(i) + "' value='ON'";
      if (relayState[i] == HIGH) {
        paginaHTML += " checked";
      }
      paginaHTML += "><br>";
    }

    paginaHTML += "<input type='submit' value='Imposta'>";
    paginaHTML += "</form>";  // Chiudi il form

    paginaHTML += "</body></html>";
    server.send(200, "text/html", paginaHTML.c_str());  // Converti la stringa in array di caratteri
  });

  server.on("/setrelays", HTTP_POST, []() {
    Serial.println("Richiesta POST ricevuta");

    // Leggi i dati inviati e aggiorna lo stato dei relè
    for (int i = 0; i < 16; i++) {
      String paramName = "rele" + String(i);
      String releValue = server.arg(paramName);

      // In base al valore ricevuto, aggiorna lo stato del relè corrispondente
      if (releValue == "ON") {
        relayState[i] = HIGH;
        mcp.digitalWrite(i, HIGH);  // Accendi il relè
      } else {
        relayState[i] = LOW;
        mcp.digitalWrite(i, LOW);  // Spegni il relè
      }

      // Stampa lo stato di ciascun relè
      Serial.print("Relè " + String(i) + ": ");
      Serial.println(relayState[i] == HIGH ? "ON" : "OFF");
    }

    server.send(200, "text/plain", "Stato dei relè impostato con successo");
  });

  server.begin();
}

void loop() {
  server.handleClient();

  if (ntpDateTime.year != 0 && ntpDateTime.month != 0 && ntpDateTime.day != 0) {
    rtc.setDateTime(&ntpDateTime);
    Serial.println("Orario sincronizzato con Internet");
    lastSyncTime = millis();
  }

  // Verifica se l'orologio è stato sincronizzato o se è passato un'ora dall'ultima sincronizzazione
  if (!clockSynced || (millis() - lastSyncTime) > 3600000) {
    if (!isAPMode) {
      // Se non è in modalità AP, tenta di connettersi a Internet e sincronizzare l'orologio
      syncClockWithNTP();
    }
    clockSynced = true;
  }

  // Leggi l'ora corrente dall'RTC
  Ds1302::DateTime now;
  rtc.getDateTime(&now);

  // Stampa l'orologio in ogni ciclo del loop
  Serial.print("20");
  Serial.print(now.year);  // 00-99
  Serial.print('-');
  if (now.month < 10) Serial.print('0');
  Serial.print(now.month);  // 01-12
  Serial.print('-');
  if (now.day < 10) Serial.print('0');
  Serial.print(now.day);  // 01-31
  Serial.print(' ');
  if (now.hour < 10) Serial.print('0');
  Serial.print(now.hour);  // 00-23
  Serial.print(':');
  if (now.minute < 10) Serial.print('0');
  Serial.print(now.minute);  // 00-59
  Serial.print(':');
  if (now.second < 10) Serial.print('0');
  Serial.print(now.second);  // 00-59
  Serial.println();
}

void syncClockWithNTP() {
  WiFiUDP udp;
  udp.begin(123);  // La porta 123 è quella standard per NTP
  udp.beginPacket(ntpServer, 123);
  udp.write(0xE3);  // Richiesta NTP
  udp.endPacket();

  unsigned long startTime = millis();
  while (millis() - startTime < 5000) {  // Aspetta fino a 5 secondi per la risposta
    if (udp.parsePacket()) {
      uint8_t packetBuffer[48];
      udp.read(packetBuffer, 48);
      unsigned long highWord = word(packetBuffer[43], packetBuffer[42]);
      unsigned long lowWord = word(packetBuffer[41], packetBuffer[40]);
      unsigned long secsSince1900 = highWord << 16 | lowWord;
      const unsigned long seventyYears = 2208988800UL;
      unsigned long epoch = secsSince1900 - seventyYears;

      // Imposta l'orologio RTC
      Ds1302::DateTime dt;
      dt.second = epoch % 60;
      epoch /= 60;
      dt.minute = epoch % 60;
      epoch /= 60;
      dt.hour = epoch % 24;
      epoch /= 24;
      dt.day = epoch % 31 + 1;
      epoch /= 31;
      dt.month = epoch % 12 + 1;
      epoch /= 12;
      dt.year = epoch + 1970;
      dt.dow = calculateDayOfWeek(dt.day, dt.month, dt.year);

      // Memorizza la data restituita dal server NTP
      ntpDateTime = dt;

      Serial.println("Orario sincronizzato con Internet");
      lastSyncTime = millis();

      break;
    }
    delay(100);
  }
}

int calculateDayOfWeek(int day, int month, int year) {
  if (month < 3) {
    month += 12;
    year -= 1;
  }
  int k = year % 100;
  int j = year / 100;
  int dow = (day + 13 * (month + 1) / 5 + k + k / 4 + j / 4 - 2 * j) % 7;
  if (dow == 0) {
    return 7;  // Converti 0 a 7 (Lunedì a Domenica)
  } else {
    return dow;
  }
}
