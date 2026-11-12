/*
 * trechter.cpp
 *
 * Beide motoren krijgen exact dezelfde relatieve beweging (zelfde aantal
 * stappen, snelheid en versnelling) direct na elkaar, dus ze lopen gelijk.
 * Het aantal stappen is zo gekozen dat een beweging TRECHTER_LOOPTIJD_MS
 * duurt: trapezium-profiel, afstand = v * (T - v/a).
 */
#include "trechter.h"

#include "config.h"
#include "safety.h"

namespace {

int32_t slagStappen() {
  float t = TRECHTER_LOOPTIJD_MS / 1000.0f;
  float rampTijd = (float)TRECHTER_SNELHEID_HZ / TRECHTER_VERSNELLING;
  return lroundf(TRECHTER_SNELHEID_HZ * (t - rampTijd));
}

}  // namespace

bool Trechter::begin(FastAccelStepperEngine &engine) {
  _m1 = engine.stepperConnectToPin(PIN_STEP_TRECHTER1);
  _m2 = engine.stepperConnectToPin(PIN_STEP_TRECHTER2);
  if (_m1 == nullptr || _m2 == nullptr) return false;

  _m1->setDirectionPin(PIN_DIR_TRECHTER1, true);
  _m2->setDirectionPin(PIN_DIR_TRECHTER2, true);
  for (FastAccelStepper *m : {_m1, _m2}) {
    m->setSpeedInHz(TRECHTER_SNELHEID_HZ);
    m->setAcceleration(TRECHTER_VERSNELLING);
  }
  // EN is gedeeld en wordt door Safety beheerd: bewust geen setEnablePin().
  return true;
}

void Trechter::start() {
  if (_m1 == nullptr || _m2 == nullptr) return;
  if (!Safety::zetDrivers(true)) {
    Serial.println(F("Trechter: noodstop actief, niet gestart."));
    return;
  }
  if (Safety::s1Geraakt() || Safety::s2Geraakt()) {
    Serial.println(F("Trechter: eindschakelaar actief (geraakt of kabel los), niet gestart."));
    _fase = Fase::FOUT;
    return;
  }
  Serial.println(F("Trechter: start cyclus"));
  beweeg(true);
  _fase = Fase::HEEN;
}

void Trechter::stop() {
  if (_m1 == nullptr) return;
  _m1->stopMove();
  _m2->stopMove();
  _fase = Fase::UIT;
  Serial.println(F("Trechter: gestopt"));
}

void Trechter::beweeg(bool heen) {
  int32_t stappen = slagStappen();
  int32_t d1 = (heen == TRECHTER_HEEN_HOOG) ? stappen : -stappen;
  int32_t d2 = TRECHTER_MOTOR2_OMGEKEERD ? -d1 : d1;
  // Direct na elkaar starten voor een gelijktijdige beweging
  auto r1 = _m1->move(d1);
  auto r2 = _m2->move(d2);
  if (r1 != MOVE_OK || r2 != MOVE_OK) {
    Serial.printf("!! Trechter: move mislukt (motor1=%d, motor2=%d)\n", (int)r1, (int)r2);
  }
  _sinds = millis();
}

bool Trechter::staatStil() const {
  return !_m1->isRunning() && !_m2->isRunning();
}

void Trechter::noodStop(const char *reden) {
  _m1->forceStop();
  _m2->forceStop();
  _fase = Fase::FOUT;
  Serial.printf("!! Trechter gestopt: %s\n", reden);
}

void Trechter::update() {
  if (_m1 == nullptr || _fase == Fase::UIT || _fase == Fase::FOUT) return;

  if (Safety::vergrendeld()) {
    noodStop("noodstop / geen 24 V");
    return;
  }
  if (Safety::s1Geraakt() || Safety::s2Geraakt()) {
    noodStop("eindschakelaar geraakt (of kabel los)");
    return;
  }

  switch (_fase) {
    case Fase::HEEN:
      if (staatStil()) {
        Serial.printf("Trechter: heen klaar na %lu ms, wacht %lu s\n",
                      (unsigned long)(millis() - _sinds), TRECHTER_WACHT_UIT_MS / 1000UL);
        _fase = Fase::WACHT_UIT;
        _sinds = millis();
      }
      break;

    case Fase::WACHT_UIT:
      if (millis() - _sinds >= TRECHTER_WACHT_UIT_MS) {
        beweeg(false);
        _fase = Fase::TERUG;
      }
      break;

    case Fase::TERUG:
      if (staatStil()) {
        _fase = Fase::WACHT_THUIS;
        _sinds = millis();
        Serial.printf("Trechter: terug op beginpositie, wacht %lu s\n",
                      TRECHTER_WACHT_THUIS_MS / 1000UL);
      }
      break;

    case Fase::WACHT_THUIS:
      if (millis() - _sinds >= TRECHTER_WACHT_THUIS_MS) {
        beweeg(true);
        _fase = Fase::HEEN;
      }
      break;

    default:
      break;
  }
}

const char *Trechter::toestandNaam() const {
  switch (_fase) {
    case Fase::UIT:         return "UIT";
    case Fase::HEEN:        return "HEEN";
    case Fase::WACHT_UIT:   return "WACHT (uit)";
    case Fase::TERUG:       return "TERUG";
    case Fase::WACHT_THUIS: return "WACHT (thuis)";
    case Fase::FOUT:        return "GESTOPT";  }
  return "?";
}
