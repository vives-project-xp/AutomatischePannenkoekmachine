/*
 * main.cpp
 *
 * Opstarten en hoofdlus van de automatische pannenkoekmachine. In deze stap
 * doen alleen de twee bandmotoren (motor 1 en 2) iets: zodra de AAN/UIT-
 * schakelaar van UIT naar AAN gaat, homen ze, rijden naar positie 1 en 2 en
 * keren terug naar de startpositie. Dit herhaalt zich elke minuut.
 * Op positie 1 en 2 opent en sluit de trechter (motor 3, trechter_motor.cpp).
 * Motor 4 (spatel): omhoog tot de eindschakelaar op GPIO21 (spatel_motor.cpp).
 * Met TEST_ALLEEN_SPATEL (config.h) werkt alleen de spatel en staan motor 1-3 stil.
 * De servo doet nog niets (pin vast LAAG).
 * Seriele commando's (115200 baud): stop, status, reset.
 * De loop blokkeert nooit.
 */
#include <Arduino.h>
#include <FastAccelStepper.h>

#include "band_home.h"
#include "config.h"
#include "safety.h"
#include "spatel_motor.h"
#include "trechter_motor.h"

FastAccelStepperEngine engine;
BandHome band;

static void voerUit(char *regel) {
  char *cmd = strtok(regel, " \t");
  if (cmd == nullptr) return;
  for (char *p = cmd; *p; p++) *p = tolower(*p);

  if (!strcmp(cmd, "stop")) {
    band.stop();
    SpatelMotor::stop();
  } else if (!strcmp(cmd, "status")) {
    Serial.printf("Schakelaar: %s\n", Safety::aanUitAan() ? "AAN" : "UIT");
    Serial.printf("Band     : %s\n", band.faseNaam());
    Serial.printf("Spatel   : %s\n", SpatelMotor::faseNaam());
    Serial.printf("Spatel-S (21): %s\n", Safety::spatelGeraakt() ? "GERAAKT / kabel los" : "vrij");
    Serial.printf("S1 (16)  : %s\n", Safety::s1Geraakt() ? "GERAAKT / kabel los" : "vrij");
    Serial.printf("S2 (17)  : %s\n", Safety::s2Geraakt() ? "GERAAKT / kabel los" : "vrij");
    Serial.printf("Drivers  : %s\n", Safety::driversAan() ? "aan" : "uit");
  } else if (!strcmp(cmd, "reset")) {
    if (Safety::reset()) {
      Serial.println(F("Reset ok, zet de schakelaar UIT en weer AAN."));
    } else {
      Serial.println(F("Reset geweigerd: nog geen 24 V."));
    }
  } else {
    Serial.println(F("Commando's: stop, status, reset (starten met de AAN/UIT-schakelaar)"));
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
  Serial.println(F("=== Automatische pannenkoekmachine - bandmotoren ==="));

  Safety::begin();  // als eerste: drivers uit en noodstop controleren

  // Servo vast LAAG: die doet voorlopig niets.
  digitalWrite(PIN_SERVO, LOW);
  pinMode(PIN_SERVO, OUTPUT);

  engine.init();
  if (!band.begin(engine)) {
    Serial.println(F("!! FOUT: bandmotoren konden niet worden aangemaakt."));
    return;
  }
  if (!TrechterMotor::begin(engine)) {
    Serial.println(F("!! FOUT: trechtermotor kon niet worden aangemaakt."));
    return;
  }
  if (!SpatelMotor::begin(engine)) {
    Serial.println(F("!! FOUT: spatelmotor kon niet worden aangemaakt."));
    return;
  }

  if (Safety::aanUitAan()) {
    Serial.println(F("Schakelaar staat AAN: zet hem eerst UIT en dan weer AAN om te starten."));
  } else {
    Serial.println(F("Wacht op de AAN/UIT-schakelaar (zet hem AAN om te starten)."));
  }
}

// Start alleen bij de overgang UIT -> AAN; bij AAN -> UIT stopt alles meteen.
static void verwerkSchakelaar() {
  static bool vorigAan = Safety::aanUitAan();
  bool aan = Safety::aanUitAan();
  if (aan == vorigAan) return;
  vorigAan = aan;

  if (aan) {
    Serial.println(F("Schakelaar AAN: start"));
    if (TEST_ALLEEN_SPATEL) {
      // Alleen de spatel: drivers zelf aanzetten, de band start niet
      if (Safety::zetDrivers(true)) SpatelMotor::start();
    } else {
      band.start();
    }
  } else {
    band.stop();
    SpatelMotor::stop();
    Safety::zetDrivers(false);  // drivers uit: alles staat stil
    Serial.println(F("Schakelaar UIT: alles gestopt, drivers uit"));
  }
}

void loop() {
  Safety::update();
  verwerkSchakelaar();
  band.update();
  TrechterMotor::update();
  SpatelMotor::update();
  leesSerieel();
}
