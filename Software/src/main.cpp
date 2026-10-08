/*
 * main.cpp
 *
 * Opstarten en hoofdlus van de automatische pannenkoekmachine, plus de
 * seriele testinterface (115200 baud). In deze stap wordt alleen de bandas
 * aangestuurd. De loop blokkeert nooit: veiligheid en as worden bij elke
 * doorgang bijgewerkt en seriele tekens worden een voor een ingelezen.
 */
#include <Arduino.h>
#include <FastAccelStepper.h>

#include "band_axis.h"
#include "config.h"
#include "safety.h"

FastAccelStepperEngine engine;  // gedeeld door alle (toekomstige) assen
BandAxis band;

// ---------------------------------------------------------------------------
// Seriele commando's
// ---------------------------------------------------------------------------
static void toonHelp() {
  Serial.println(F("Commando's:"));
  Serial.println(F("  home            homing op S1"));
  Serial.println(F("  goto <mm>       absolute positie"));
  Serial.println(F("  move <mm>       relatief, mag negatief"));
  Serial.println(F("  speed <mm/s>    maximale snelheid"));
  Serial.println(F("  accel <mm/s2>   versnelling"));
  Serial.println(F("  stop            rustig afremmen"));
  Serial.println(F("  enable/disable  drivers aan/uit"));
  Serial.println(F("  status          toestand tonen"));
  Serial.println(F("  reset           fout wissen"));
}

static void toonStatus() {
  Serial.printf("Toestand : %s\n", band.toestandNaam());
  Serial.printf("Positie  : %.2f mm\n", band.positieMm());
  Serial.printf("Gehomed  : %s\n", band.isGehomed() ? "ja" : "nee");
  Serial.printf("S1       : %s\n", Safety::s1Geraakt() ? "GERAAKT / kabel los" : "vrij");
  Serial.printf("Noodstop : %s\n",
                !Safety::spanningAanwezig() ? "ACTIEF (geen 24 V)"
                : Safety::vergrendeld()     ? "24 V ok, wacht op 'reset'"
                                            : "ok (24 V aanwezig)");
  Serial.printf("Enabled  : %s\n", Safety::driversAan() ? "ja" : "nee");
  Serial.printf("Snelheid : %.1f mm/s, versnelling %.1f mm/s2\n",
                band.snelheid(), band.versnelling());
  if (band.toestand() == BandAxis::Toestand::ERROR) {
    Serial.printf("Fout     : %s\n", band.foutTekst());
  }
}

// Leest het getal achter een commando. Komma en punt zijn allebei toegestaan.
static bool leesGetal(char *tekst, float &waarde) {
  if (tekst == nullptr) return false;
  for (char *p = tekst; *p; p++) {
    if (*p == ',') *p = '.';
  }
  char *einde;
  waarde = strtof(tekst, &einde);
  return einde != tekst && *einde == '\0' && isfinite(waarde);
}

static void voerUit(char *regel) {
  char *cmd = strtok(regel, " \t");
  char *arg = strtok(nullptr, " \t");
  if (cmd == nullptr) return;  // lege regel
  for (char *p = cmd; *p; p++) *p = tolower(*p);

  float getal = 0;
  bool isGetalCommando = !strcmp(cmd, "goto") || !strcmp(cmd, "move") ||
                         !strcmp(cmd, "speed") || !strcmp(cmd, "accel");
  if (isGetalCommando && !leesGetal(arg, getal)) {
    Serial.printf("Gebruik: %s <getal>\n", cmd);
    return;
  }

  if (!strcmp(cmd, "home")) {
    band.home();
  } else if (!strcmp(cmd, "goto")) {
    if (band.gaNaar(getal)) Serial.printf("Naar %.2f mm\n", getal);
  } else if (!strcmp(cmd, "move")) {
    if (band.verplaats(getal)) Serial.printf("Verplaats %+.2f mm\n", getal);
  } else if (!strcmp(cmd, "speed")) {
    if (band.zetSnelheid(getal)) Serial.printf("Snelheid = %.1f mm/s\n", getal);
  } else if (!strcmp(cmd, "accel")) {
    if (band.zetVersnelling(getal)) Serial.printf("Versnelling = %.1f mm/s2\n", getal);
  } else if (!strcmp(cmd, "stop")) {
    band.stop();
    Serial.println(F("Stop: afremmen"));
  } else if (!strcmp(cmd, "enable")) {
    if (Safety::zetDrivers(true)) {
      Serial.println(F("Drivers AAN"));
    } else {
      Serial.println(F("Geweigerd: noodstop actief, herstel 24 V en typ 'reset'."));
    }
  } else if (!strcmp(cmd, "disable")) {
    band.vrijgeven();  // eerst de pulsen stoppen, dan pas de drivers uit
    Safety::zetDrivers(false);
    Serial.println(F("Drivers UIT (band moet opnieuw gehomed worden)"));
  } else if (!strcmp(cmd, "status")) {
    toonStatus();
  } else if (!strcmp(cmd, "reset")) {
    // Eerst de noodstop-vergrendeling: lukt alleen als de 24 V terug is
    if (!Safety::reset()) {
      Serial.println(F("Reset geweigerd: nog geen 24 V (noodstop ingedrukt?)."));
    } else if (band.resetFout()) {
      Serial.println(F("Fout gewist. Typ 'enable' en daarna 'home'."));
    } else {
      Serial.println(F("Reset mislukt."));
    }
  } else if (!strcmp(cmd, "help") || !strcmp(cmd, "?")) {
    toonHelp();
  } else {
    Serial.printf("Onbekend commando '%s', typ 'help'.\n", cmd);
  }
}

// Verzamelt tekens tot een volledige regel, zonder te wachten.
static void leesSerieel() {
  static char buffer[48];
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

// ---------------------------------------------------------------------------
// Setup en loop
// ---------------------------------------------------------------------------
void setup() {
  Serial.begin(SERIEEL_BAUD);
  Serial.println();
  Serial.println(F("=== Automatische pannenkoekmachine - bandas ==="));

  Safety::begin();  // als eerste: drivers uit en noodstop controleren

  engine.init();
  if (!band.begin(engine)) {
    Serial.println(F("!! FOUT: stappenmotor BAND kon niet worden aangemaakt."));
  }

  toonHelp();
}

void loop() {
  Safety::update();  // noodstop en eindschakelaars
  band.update();     // toestandsmachine van de band
  leesSerieel();     // testinterface
}
