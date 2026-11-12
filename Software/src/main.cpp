/*
 * main.cpp
 *
 * Opstarten en hoofdlus van de automatische pannenkoekmachine. In deze stap
 * draaien alleen de twee trechtermotoren (gelijktijdig, zelfde beweging):
 * 3 s heen, 5 s wachten, 3 s terug, 15 s wachten, herhalen.
 * Seriele commando's (115200 baud): start, stop, status, reset.
 * De loop blokkeert nooit.
 */
#include <Arduino.h>
#include <FastAccelStepper.h>

#include "config.h"
#include "safety.h"
#include "trechter.h"

FastAccelStepperEngine engine;
Trechter trechter;

static void voerUit(char *regel) {
  char *cmd = strtok(regel, " \t");
  if (cmd == nullptr) return;
  for (char *p = cmd; *p; p++) *p = tolower(*p);

  if (!strcmp(cmd, "start")) {
    trechter.start();
  } else if (!strcmp(cmd, "stop")) {
    trechter.stop();
  } else if (!strcmp(cmd, "status")) {
    Serial.printf("Trechter : %s\n", trechter.toestandNaam());
    Serial.printf("S1 (16)  : %s\n", Safety::s1Geraakt() ? "GERAAKT / kabel los" : "vrij");
    Serial.printf("S2 (17)  : %s\n", Safety::s2Geraakt() ? "GERAAKT / kabel los" : "vrij");
    Serial.printf("Drivers  : %s\n", Safety::driversAan() ? "aan" : "uit");
  } else if (!strcmp(cmd, "reset")) {
    if (Safety::reset()) {
      Serial.println(F("Reset ok, typ 'start'."));
    } else {
      Serial.println(F("Reset geweigerd: nog geen 24 V."));
    }
  } else {
    Serial.println(F("Commando's: start, stop, status, reset"));
  }
}

static void leesSerieel() {
  static char buffer[32];
  static size_t lengte = 0;

  while (Serial.available() > 0) {
    char c = (char)Serial.read();
    if (c == '\r' || c == '\n') {
      buffer[lengte] = '\0';
      lengte = 0;
      voerUit(buffer);
    } else if (lengte < sizeof(buffer) - 1) {
      buffer[lengte++] = c;
    }
  }
}

void setup() {
  Serial.begin(SERIEEL_BAUD);
  Serial.println();
  Serial.println(F("=== Automatische pannenkoekmachine - trechter ==="));

  Safety::begin();  // als eerste: drivers uit en noodstop controleren

  // STEP/DIR van de andere motoren vast LAAG: een zwevende STEP-ingang kan
  // door storing stappen geven. Zo draaien alleen de trechtermotoren.
  for (uint8_t pin : {PIN_STEP_SPATEL, PIN_DIR_SPATEL, PIN_STEP_DRAAI, PIN_DIR_DRAAI}) {
    digitalWrite(pin, LOW);
    pinMode(pin, OUTPUT);
  }

  engine.init();
  if (!trechter.begin(engine)) {
    Serial.println(F("!! FOUT: trechtermotoren konden niet worden aangemaakt."));
    return;
  }

  trechter.start();  // cyclus start meteen na het opstarten
}

void loop() {
  Safety::update();
  trechter.update();
  leesSerieel();
}
