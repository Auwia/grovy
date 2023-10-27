/* * Author: Massimo Manganiello * first development: 27/10/2023 * version: 1.0 */
#include <Adafruit_MCP23X08.h>
#include <Adafruit_MCP23X17.h>
#include <Adafruit_MCP23XXX.h>

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <RTClib.h>
#include <Wire.h>

const char* ssidRouter = "UPC2522560";
const char* passwordRouter = "7FxuryjpTtus";

const char* ssidAP = "Prototype_001";
const char* passwordAP = "PasswordAP";

ESP8266WebServer server(80);
RTC_DS3231 rtc;

bool isAPMode = true;

Adafruit_MCP23X17 mcp;

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

  // uncomment appropriate mcp.begin
  if (!mcp.begin_I2C()) {
    Serial.println("Error.");
    while (1)
      ;
  }

  for (int i = 0; i < 16; i++) {
    // configure pin for output
    mcp.pinMode(i, OUTPUT);
  }
  /*
  // Inizializza l'RTC
  rtc.begin();
  // Imposta l'orario e la data iniziali se necessario
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

*/

  server.on("/", HTTP_GET, []() {
    String paginaHTML = "<html><body>";
    paginaHTML += "<h1>Controllo dei Rele</h1>";

    paginaHTML += "<form action='/setrelays' method='post'>";  // Apri il form per la richiesta POST

    for (int i = 0; i < 16; i++) {
      paginaHTML += "Rele " + String(i) + ": ";
      paginaHTML += "<input type='checkbox' name='rele" + String(i) + "' value='0'><br>";
    }

    paginaHTML += "<input type='submit' value='Imposta'>";
    paginaHTML += "</form>";  // Chiudi il form

    paginaHTML += "</body></html>";
    server.send(200, "text/html", paginaHTML.c_str());  // Converti la stringa in array di caratteri
  });

  server.on("/setrelays", HTTP_POST, []() {
    Serial.println("Richiesta POST ricevuta");  // Aggiungi un messaggio di debug

    // Leggi i dati inviati e imposta i relè corrispondenti
    for (int i = 0; i < 16; i++) {
      String paramName = "rele" + String(i);
      int releValue = server.arg(paramName).toInt();

      mcp.digitalWrite(i, releValue);


      // Stampa lo stato di ciascun relè
      Serial.print("Rele " + String(i) + ": ");
      Serial.println(releValue == HIGH ? "ON" : "OFF");
    }

    server.send(200, "text/plain", "Stato dei rele impostato con successo");
  });


  server.begin();
}

void loop() {

  WiFi.softAPIP();

  delay(500);

  server.handleClient();

  delay(500);

  /* 
  // Leggi l'ora corrente dall'RTC
  DateTime now = rtc.now();
  
  // Esegui il controllo dell'orario e attiva/disattiva i Rele in base al tuo programma
  if (now.hour() >= 8 && now.hour() < 12) {
    // Attiva i Rele desiderati durante l'orario specificato
    digitalWrite(0, HIGH); // Ad esempio, attiva il Rele collegato al pin 0
  } else {
    // Disattiva i Rele al di fuori dell'orario specificato
    digitalWrite(0, LOW); // Ad esempio, disattiva il Rele collegato al pin 0
  }
  */
}
