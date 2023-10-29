/* * Author: Massimo Manganiello * first development: 27/10/2023 * version: 1.0 */
#include <Adafruit_MCP23X08.h>
#include <Adafruit_MCP23X17.h>
#include <Adafruit_MCP23XXX.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <WiFiUdp.h>
#include <DS1302.h>

const char* ssidRouter = "UPC2522560";
const char* passwordRouter = "7FxuryjpTtus";

const char* ssidAP = "Prototype_001";
const char* passwordAP = "PasswordAP";

ESP8266WebServer server(80);

bool isAPMode = true;

Adafruit_MCP23X17 mcp;

// Init the DS1302
DS1302 rtc(0, 2, 14);

// Dichiarazione di un array per memorizzare lo stato dei rele
bool relayState[16] = { LOW };  // Inizialmente tutti i rele spenti

void setup() {
  // Set the clock to run-mode, and disable the write protection
  rtc.halt(false);
  rtc.writeProtect(false);

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

  server.on("/", HTTP_GET, []() {
    String paginaHTML = "<html><body>";
    paginaHTML += "<h1>Controllo dei Rele</h1>";

    paginaHTML += "<form action='/setrelays' method='post'>";  // Apri il form per la richiesta POST

    for (int i = 0; i < 16; i++) {
      paginaHTML += "Rele " + String(i) + ": ";
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

    // Leggi i dati inviati e aggiorna lo stato dei rele
    for (int i = 0; i < 16; i++) {
      String paramName = "rele" + String(i);
      String releValue = server.arg(paramName);

      // In base al valore ricevuto, aggiorna lo stato del rele corrispondente
      if (releValue == "ON") {
        relayState[i] = HIGH;
        mcp.digitalWrite(i, HIGH);  // Accendi il rele
      } else {
        relayState[i] = LOW;
        mcp.digitalWrite(i, LOW);  // Spegni il rele
      }

      // Stampa lo stato di ciascun rele
      Serial.print("Rele " + String(i) + ": ");
      Serial.println(relayState[i] == HIGH ? "ON" : "OFF");
    }

    server.send(200, "text/plain", "Stato dei rele impostato con successo");
  });

  server.begin();
}

void loop() {
  server.handleClient();

  // Send Day-of-Week
  Serial.print(rtc.getDOWStr());
  Serial.print(" ");
  
  // Send date
  Serial.print(rtc.getDateStr());
  Serial.print(" -- ");

  // Send time
  Serial.println(rtc.getTimeStr());
  
  // Wait one second before repeating :)
  delay (1000);
}
