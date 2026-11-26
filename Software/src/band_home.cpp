/*
 * band_home.cpp
 *
 * Beide motoren krijgen altijd dezelfde opdracht. Naar de startpositie en
 * terug draaien ze continu naar rechts tot een van de eindschakelaars (NC,
 * HIGH = ingedrukt of kabel los) raakt; dan stoppen ze allebei meteen.
 * De bewegingen weg van de schakelaar zijn vaste aantallen stappen die samen
 * BAND_LOOPTIJD_MS duren (trapeziumprofiel: afstand = v * (T - v/a)).
 */
#include "band_home.h"

#include "config.h"
#include "safety.h"
#include "trechter_motor.h"

namespace {

bool schakelaarMotor1() {
  return BAND_SCHAKELAARS_GEWISSELD ? Safety::s2Geraakt() : Safety::s1Geraakt();
}

bool schakelaarMotor2() {
  return BAND_SCHAKELAARS_GEWISSELD ? Safety::s1Geraakt() : Safety::s2Geraakt();
}

bool eindschakelaarGeraakt() { return schakelaarMotor1() || schakelaarMotor2(); }

int32_t wegStappen() {
  float t = BAND_LOOPTIJD_MS / 1000.0f;
  float rampTijd = (float)BAND_HOME_SNELHEID_HZ / BAND_HOME_VERSNELLING;
  return lroundf(BAND_HOME_SNELHEID_HZ * (t - rampTijd));
}

}  // namespace

bool BandHome::begin(FastAccelStepperEngine &engine) {
  _m1 = engine.stepperConnectToPin(PIN_STEP_BAND);
  _m2 = engine.stepperConnectToPin(PIN_STEP_BAND2);
  if (_m1 == nullptr || _m2 == nullptr) return false;

  // runForward() = naar rechts (naar de eindschakelaar)
  _m1->setDirectionPin(PIN_DIR_BAND, BAND_RECHTS_DIR_HOOG);
  _m2->setDirectionPin(PIN_DIR_BAND2, BAND_RECHTS_DIR_HOOG != BAND_MOTOR2_OMGEKEERD);
  for (FastAccelStepper *m : {_m1, _m2}) {
    m->setSpeedInHz(BAND_HOME_SNELHEID_HZ);
    m->setAcceleration(BAND_HOME_VERSNELLING);
  }
  // EN is gedeeld en wordt door Safety beheerd: bewust geen setEnablePin().
  return true;
}

void BandHome::start() {
  if (_m1 == nullptr || _m2 == nullptr) return;
  if (!Safety::zetDrivers(true)) {
    Serial.println(F("Band: noodstop actief, niet gestart."));
    return;
  }
  _sinds = millis();
  _cyclusStart = _sinds;
  if (eindschakelaarGeraakt()) {
    // Staat al op een schakelaar: dit is dus al de startpositie
    _m1->forceStop();
    _m2->forceStop();
    _m1->setCurrentPosition(0);
    _m2->setCurrentPosition(0);
    _fase = Fase::WACHT_START;
    Serial.printf("Band: staat al op startpositie, wacht %lu s\n",
                  BAND_WACHT_START_MS / 1000UL);
    return;
  }
  Serial.println(F("Band: naar rechts draaien tot een eindschakelaar..."));
  _m1->runForward();
  _m2->runForward();
  _fase = Fase::ZOEKT;
}

void BandHome::stop() {
  if (_m1 == nullptr) return;
  _m1->forceStop();
  _m2->forceStop();
  TrechterMotor::stop();
  _fase = Fase::UIT;
  Serial.println(F("Band: gestopt"));
}

void BandHome::fout(const char *reden) {
  _m1->forceStop();
  _m2->forceStop();
  TrechterMotor::stop();
  _fase = Fase::FOUT;
  Serial.printf("!! Band gestopt: %s\n", reden);
}

void BandHome::beweegWeg() {
  // Weg van de schakelaar = tegengestelde richting van runForward()
  int32_t stappen = -wegStappen();
  // Direct na elkaar starten voor een gelijktijdige beweging
  auto r1 = _m1->move(stappen);
  auto r2 = _m2->move(stappen);
  if (r1 != MOVE_OK || r2 != MOVE_OK) {
    fout("move mislukt");
    return;
  }
  _schakelaarVrij = false;
  _sinds = millis();
}

bool BandHome::staatStil() const {
  return !_m1->isRunning() && !_m2->isRunning();
}

// Op een positie: verder als de wachttijd voorbij is EN de trechter weer dicht is
bool BandHome::wachtKlaar() {
  if (!TrechterMotor::isKlaar()) {
    if (!TrechterMotor::isBezig()) fout("trechtermotor is gestopt");
    return false;
  }
  return millis() - _sinds >= BAND_WACHT_UIT_MS;
}

void BandHome::update() {
  if (_m1 == nullptr) return;

  // Klaar: de volgende cyclus start een minuut na het begin van de vorige
  if (_fase == Fase::KLAAR) {
    if (!Safety::vergrendeld() && millis() - _cyclusStart >= BAND_CYCLUS_MS) {
      Serial.println(F("Band: nieuwe cyclus"));
      start();
    }
    return;
  }
  if (_fase == Fase::UIT || _fase == Fase::FOUT) return;

  if (Safety::vergrendeld()) {
    fout("noodstop / geen 24 V");
    return;
  }

  switch (_fase) {
    case Fase::ZOEKT:
    case Fase::TERUG:
      // Een van de twee schakelaars raakt: allebei de motoren stoppen meteen
      if (eindschakelaarGeraakt()) {
        _m1->forceStop();
        _m2->forceStop();
        _m1->setCurrentPosition(0);
        _m2->setCurrentPosition(0);
        Serial.printf("Band: startpositie bereikt na %lu ms (S1=%d, S2=%d)\n",
                      (unsigned long)(millis() - _sinds), (int)Safety::s1Geraakt(),
                      (int)Safety::s2Geraakt());
        if (_fase == Fase::ZOEKT) {
          Serial.printf("Band: wacht %lu s\n", BAND_WACHT_START_MS / 1000UL);
          _fase = Fase::WACHT_START;
          _sinds = millis();
        } else {
          Serial.printf("Band: cyclus klaar, volgende over %lu s\n",
                        (unsigned long)((BAND_CYCLUS_MS - (millis() - _cyclusStart)) / 1000UL));
          _fase = Fase::KLAAR;
        }
      } else if (millis() - _sinds > BAND_HOME_MAX_MS) {
        fout("eindschakelaar niet bereikt binnen de maximale tijd");
      }
      break;

    case Fase::WACHT_START:
      if (millis() - _sinds >= BAND_WACHT_START_MS) {
        Serial.println(F("Band: beweging 1 weg van de schakelaar"));
        beweegWeg();
        if (_fase != Fase::FOUT) _fase = Fase::WEG_1;
      }
      break;

    case Fase::WEG_1:
    case Fase::WEG_2:
      // Aan het begin staat de motor nog op de schakelaar: die komt pas na een
      // stukje vrij. Raakt hij daarna opnieuw, dan klopt er iets niet.
      if (!eindschakelaarGeraakt()) {
        _schakelaarVrij = true;
      } else if (_schakelaarVrij) {
        fout("eindschakelaar geraakt tijdens beweging weg (of kabel los)");
        break;
      }
      if (staatStil()) {
        if (!_schakelaarVrij) {
          fout("eindschakelaar bleef ingedrukt tijdens beweging weg (beweging te kort?)");
          break;
        }
        // Band staat stil op de positie: nu opent en sluit de trechter
        Serial.printf("Band: positie bereikt, trechter opent (wacht min. %lu s)\n",
                      BAND_WACHT_UIT_MS / 1000UL);
        _fase = (_fase == Fase::WEG_1) ? Fase::WACHT_1 : Fase::WACHT_2;
        _sinds = millis();
        if (!TrechterMotor::start()) fout("trechtermotor kon niet starten");
      }
      break;

    case Fase::WACHT_1:
      if (wachtKlaar()) {
        Serial.println(F("Band: beweging 2 weg van de schakelaar"));
        beweegWeg();
        if (_fase != Fase::FOUT) _fase = Fase::WEG_2;
      }
      break;

    case Fase::WACHT_2:
      if (wachtKlaar()) {
        Serial.println(F("Band: terug naar de startpositie"));
        _m1->runForward();
        _m2->runForward();
        _fase = Fase::TERUG;
        _sinds = millis();
      }
      break;

    default:
      break;
  }
}

const char *BandHome::faseNaam() const {
  switch (_fase) {
    case Fase::UIT:         return "UIT";
    case Fase::ZOEKT:       return "ZOEKT STARTPOSITIE";
    case Fase::WACHT_START: return "WACHT (startpositie)";
    case Fase::WEG_1:       return "BEWEGING 1 WEG";
    case Fase::WACHT_1:     return "WACHT (na beweging 1)";
    case Fase::WEG_2:       return "BEWEGING 2 WEG";
    case Fase::WACHT_2:     return "WACHT (na beweging 2)";
    case Fase::TERUG:       return "TERUG NAAR STARTPOSITIE";
    case Fase::KLAAR:       return "KLAAR (op startpositie)";
    case Fase::FOUT:        return "FOUT";
  }
  return "?";
}
