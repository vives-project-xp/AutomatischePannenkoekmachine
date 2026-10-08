/*
 * safety.cpp
 *
 * Implementatie van de veiligheidslaag: noodstop-vergrendeling, ontdenderen
 * van de eindschakelaars en het schakelen van de gezamenlijke EN-lijn.
 * Alles is niet-blokkerend en wordt vanuit loop() bijgewerkt.
 */
#include "safety.h"

#include "config.h"

namespace {

// Een ontdenderde schakelaar. Extra eindschakelaars (GPIO17/18) kunnen later
// gewoon als extra exemplaar worden toegevoegd.
struct Schakelaar {
  uint8_t pin;
  bool stabiel = false;   // ontdenderde toestand (true = HIGH)
  bool laatste = false;   // laatst gelezen ruwe toestand
  uint32_t sinds = 0;     // tijdstip van de laatste ruwe wijziging

  explicit Schakelaar(uint8_t p) : pin(p) {}

  void begin() {
    pinMode(pin, INPUT_PULLUP);
    stabiel = laatste = (digitalRead(pin) == HIGH);
    sinds = millis();
  }

  void update() {
    bool ruw = (digitalRead(pin) == HIGH);
    if (ruw != laatste) {
      // Niveau veranderd: ontdendertijd opnieuw starten
      laatste = ruw;
      sinds = millis();
    } else if (ruw != stabiel && millis() - sinds >= ONTDENDER_MS) {
      // Lang genoeg stabiel: nieuwe toestand overnemen
      stabiel = ruw;
    }
  }
};

Schakelaar s1{PIN_S1_HOME};

bool isVergrendeld = false;
bool driversActief = false;

void schrijfEnable(bool aan) {
  digitalWrite(PIN_EN_DRIVERS, aan ? LOW : HIGH);  // actief LAAG
  driversActief = aan;
}

}  // namespace

namespace Safety {

void begin() {
  // Eerst het niveau zetten en dan pas uitgang maken, zodat EN nooit even
  // laag wordt: de drivers blijven uit tijdens het opstarten.
  digitalWrite(PIN_EN_DRIVERS, HIGH);
  pinMode(PIN_EN_DRIVERS, OUTPUT);
  schrijfEnable(false);

  pinMode(PIN_NOODSTOP, INPUT);  // GPIO35 heeft geen interne pull-up
  s1.begin();

  update();  // staat de 24 V bij het opstarten af, dan meteen vergrendelen
}

void update() {
  s1.update();

  // Noodstop: zonder ontdendering, zodat de reactie zo snel mogelijk is.
  if (!spanningAanwezig() && !isVergrendeld) {
    schrijfEnable(false);  // EN hoog: drivers uit
    isVergrendeld = true;
    Serial.println(F("!! NOODSTOP: geen 24 V. Drivers uit, 'reset' nodig."));
  }
}

bool s1Geraakt() { return s1.stabiel; }  // NC-contact: HIGH = geraakt/kabel los

bool spanningAanwezig() { return digitalRead(PIN_NOODSTOP) == HIGH; }

bool vergrendeld() { return isVergrendeld; }

bool reset() {
  if (!spanningAanwezig()) return false;  // eerst noodstop ontgrendelen
  isVergrendeld = false;
  return true;
}

bool zetDrivers(bool aan) {
  if (aan && isVergrendeld) return false;
  schrijfEnable(aan);
  return true;
}

bool driversAan() { return driversActief; }

}  // namespace Safety
