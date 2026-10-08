/*
 * band_axis.cpp
 *
 * Logica van de bandas: omrekenen van millimeter naar stappen, de
 * toestandsmachine, de homing-sequentie op S1 en de softwarelimieten.
 * De stappulsen zelf maakt FastAccelStepper in hardware; update() kijkt
 * alleen of er iets moet gebeuren en keert meteen terug.
 */
#include "band_axis.h"

#include "config.h"
#include "safety.h"

namespace {

int32_t mmNaarStappen(float mm) { return lroundf(mm * STEPS_PER_MM); }
float stappenNaarMm(int32_t stappen) { return stappen / STEPS_PER_MM; }

const int32_t SLAG_STAPPEN = mmNaarStappen(SLAGLENGTE_MM);

}  // namespace

bool BandAxis::begin(FastAccelStepperEngine &engine) {
  _snelheid = MAX_SNELHEID_MM_S;
  _versnelling = VERSNELLING_MM_S2;

  _stepper = engine.stepperConnectToPin(PIN_STEP_BAND);
  if (_stepper == nullptr) {
    _toestand = Toestand::ERROR;
    _fout = "stappenmotor BAND kon niet worden aangemaakt";
    return false;
  }
  _stepper->setDirectionPin(PIN_DIR_BAND, DIR_HOOG_TELT_OP);
  // De EN-lijn is gedeeld met de andere drivers en wordt door Safety beheerd,
  // dus hier bewust geen setEnablePin().
  zetProfiel(_snelheid, _versnelling);
  return true;
}

// ---------------------------------------------------------------------------
// Toestandsmachine
// ---------------------------------------------------------------------------
void BandAxis::update() {
  if (_stepper == nullptr) return;

  // Noodstop heeft altijd voorrang op alles
  if (Safety::vergrendeld()) {
    if (!_noodstopVerwerkt) {
      _stepper->forceStop();  // meteen stoppen, zonder afremmen
      _gehomed = false;       // de brug kan uitgebold of verschoven zijn
      _toestand = Toestand::ERROR;
      _fout = "noodstop / geen 24 V";
      _noodstopVerwerkt = true;
    }
    return;
  }

  switch (_toestand) {
    case Toestand::HOMING:
      updateHoming();
      break;

    case Toestand::MOVING:
      if (Safety::s1Geraakt()) {
        // S1 hoort tijdens een gewone beweging nooit te schakelen
        fout("S1 geraakt tijdens beweging (of kabel los)");
      } else if (!_stepper->isRunning()) {
        _toestand = Toestand::IDLE;
        Serial.printf("Band staat stil op %.2f mm\n", positieMm());
      }
      break;

    case Toestand::IDLE:
    case Toestand::ERROR:
      break;
  }
}

// ---------------------------------------------------------------------------
// Homing
// ---------------------------------------------------------------------------
bool BandAxis::home() {
  if (!kanStarten()) return false;
  if (_toestand != Toestand::IDLE) {
    Serial.println(F("Geweigerd: band is bezig, typ eerst 'stop'."));
    return false;
  }

  _gehomed = false;
  _toestand = Toestand::HOMING;

  if (Safety::s1Geraakt()) {
    // We staan al op de schakelaar: eerst vrijrijden
    _homeFase = HomeFase::TERUGRIJDEN;
    return startRelatief(HOME_TERUGRIJ_MM, HOME_SNELHEID_SNEL_MM_S);
  }

  // Maximaal de volledige slag + marge zoeken; daarna is het een fout
  _homeFase = HomeFase::ZOEK_SNEL;
  Serial.println(F("Homing gestart..."));
  return startRelatief(-(SLAGLENGTE_MM + HOME_EXTRA_ZOEK_MM),
                       HOME_SNELHEID_SNEL_MM_S);
}

void BandAxis::updateHoming() {
  bool s1 = Safety::s1Geraakt();
  bool staatStil = !_stepper->isRunning();

  switch (_homeFase) {
    case HomeFase::ZOEK_SNEL:
      if (s1) {
        _stepper->forceStop();
        _homeFase = HomeFase::STOP_NA_SNEL;
      } else if (staatStil) {
        fout("homing: S1 niet gevonden binnen slaglengte + marge");
      }
      break;

    case HomeFase::STOP_NA_SNEL:
      if (staatStil) {
        _homeFase = HomeFase::TERUGRIJDEN;
        startRelatief(HOME_TERUGRIJ_MM, HOME_SNELHEID_SNEL_MM_S);
      }
      break;

    case HomeFase::TERUGRIJDEN:
      if (staatStil) {
        if (s1) {
          // Na 5 mm terugrijden nog steeds actief: schakelaar klemt of kabel los
          fout("homing: S1 blijft actief na terugrijden (kabel los?)");
        } else {
          _homeFase = HomeFase::ZOEK_TRAAG;
          startRelatief(-2.0f * HOME_TERUGRIJ_MM, HOME_SNELHEID_TRAAG_MM_S);
        }
      }
      break;

    case HomeFase::ZOEK_TRAAG:
      if (s1) {
        _stepper->forceStop();
        _homeFase = HomeFase::STOP_NA_TRAAG;
      } else if (staatStil) {
        fout("homing: S1 niet teruggevonden bij trage benadering");
      }
      break;

    case HomeFase::STOP_NA_TRAAG:
      if (staatStil) {
        // Schakelpunt ligt HOME_OFFSET_MM onder nul, zodat S1 op 0 vrij is
        _stepper->setCurrentPosition(-mmNaarStappen(HOME_OFFSET_MM));
        _homeFase = HomeFase::NAAR_NUL;
        startRelatief(HOME_OFFSET_MM, HOME_SNELHEID_SNEL_MM_S);
      }
      break;

    case HomeFase::NAAR_NUL:
      if (staatStil) {
        if (s1) {
          fout("homing: S1 nog actief op positie 0");
        } else {
          _gehomed = true;
          _toestand = Toestand::IDLE;
          Serial.println(F("Homing klaar, positie = 0.00 mm"));
        }
      }
      break;

    case HomeFase::AFBREKEN:
      if (staatStil) {
        _toestand = Toestand::IDLE;
        Serial.println(F("Homing afgebroken, band is niet gehomed."));
      }
      break;
  }
}

// ---------------------------------------------------------------------------
// Gewone bewegingen
// ---------------------------------------------------------------------------
bool BandAxis::gaNaar(float mm) {
  if (!kanStarten()) return false;
  return naarStappen(mmNaarStappen(mm));
}

bool BandAxis::verplaats(float mm) {
  if (!kanStarten()) return false;
  // Loopt er al een beweging, dan telt de verplaatsing vanaf het huidige doel
  int32_t basis = (_toestand == Toestand::MOVING)
                      ? _stepper->targetPos()
                      : _stepper->getCurrentPosition();
  return naarStappen(basis + mmNaarStappen(mm));
}

bool BandAxis::naarStappen(int32_t doel) {
  if (_toestand == Toestand::HOMING) {
    Serial.println(F("Geweigerd: homing is bezig."));
    return false;
  }
  if (!_gehomed) {
    Serial.println(F("Geweigerd: band is niet gehomed, typ eerst 'home'."));
    return false;
  }
  // Softwarelimieten: nooit onder 0 of boven de slaglengte
  if (doel < 0 || doel > SLAG_STAPPEN) {
    Serial.printf("Geweigerd: %.2f mm ligt buiten 0 .. %.0f mm.\n",
                  stappenNaarMm(doel), SLAGLENGTE_MM);
    return false;
  }
  if (Safety::s1Geraakt()) {
    Serial.println(F("Geweigerd: S1 is actief (geraakt of kabel los)."));
    return false;
  }

  zetProfiel(_snelheid, _versnelling);
  if (_stepper->moveTo(doel) != MOVE_OK) {
    fout("beweging kon niet worden gestart");
    return false;
  }
  _toestand = Toestand::MOVING;
  return true;
}

void BandAxis::stop() {
  if (_stepper == nullptr) return;
  if (_toestand == Toestand::HOMING) {
    _stepper->stopMove();
    _homeFase = HomeFase::AFBREKEN;
  } else if (_toestand == Toestand::MOVING) {
    _stepper->stopMove();  // afremmen met de ingestelde versnelling
  }
}

void BandAxis::vrijgeven() {
  if (_stepper == nullptr) return;
  _stepper->forceStop();
  // Zonder houdkoppel kan de brug verschuiven: de positie is niet meer zeker
  _gehomed = false;
  if (_toestand != Toestand::ERROR) _toestand = Toestand::IDLE;
}

bool BandAxis::zetSnelheid(float mmPerS) {
  if (mmPerS < 0.1f || mmPerS > SNELHEID_LIMIET_MM_S) {
    Serial.printf("Geweigerd: snelheid moet tussen 0.1 en %.0f mm/s liggen.\n",
                  SNELHEID_LIMIET_MM_S);
    return false;
  }
  _snelheid = mmPerS;
  if (_toestand == Toestand::MOVING) {
    // Ook de lopende beweging aanpassen (homing houdt zijn eigen snelheid)
    zetProfiel(_snelheid, _versnelling);
    _stepper->applySpeedAcceleration();
  }
  return true;
}

bool BandAxis::zetVersnelling(float mmPerS2) {
  if (mmPerS2 < 1.0f || mmPerS2 > VERSNELLING_LIMIET_MM_S2) {
    Serial.printf("Geweigerd: versnelling moet tussen 1 en %.0f mm/s2 liggen.\n",
                  VERSNELLING_LIMIET_MM_S2);
    return false;
  }
  _versnelling = mmPerS2;
  if (_toestand == Toestand::MOVING) {
    zetProfiel(_snelheid, _versnelling);
    _stepper->applySpeedAcceleration();
  }
  return true;
}

bool BandAxis::resetFout() {
  if (_stepper == nullptr || Safety::vergrendeld()) return false;
  _noodstopVerwerkt = false;
  _fout = "";
  if (_toestand == Toestand::ERROR) _toestand = Toestand::IDLE;
  return true;
}

// ---------------------------------------------------------------------------
// Hulpfuncties
// ---------------------------------------------------------------------------
bool BandAxis::kanStarten() {
  if (_stepper == nullptr) {
    Serial.println(F("Geweigerd: stappenmotor niet geinitialiseerd."));
    return false;
  }
  if (Safety::vergrendeld()) {
    Serial.println(F("Geweigerd: noodstop actief, herstel 24 V en typ 'reset'."));
    return false;
  }
  if (_toestand == Toestand::ERROR) {
    Serial.printf("Geweigerd: fout actief (%s), typ 'reset'.\n", _fout);
    return false;
  }
  if (!Safety::driversAan()) {
    Serial.println(F("Geweigerd: drivers staan uit, typ eerst 'enable'."));
    return false;
  }
  return true;
}

// Relatieve beweging zonder limietcontrole; alleen voor de homing.
bool BandAxis::startRelatief(float mm, float mmPerS) {
  zetProfiel(mmPerS, _versnelling);
  if (_stepper->move(mmNaarStappen(mm)) != MOVE_OK) {
    fout("homingbeweging kon niet worden gestart");
    return false;
  }
  return true;
}

void BandAxis::zetProfiel(float mmPerS, float mmPerS2) {
  _stepper->setSpeedInHz((uint32_t)mmNaarStappen(mmPerS));  // stappen/s
  _stepper->setAcceleration(mmNaarStappen(mmPerS2));        // stappen/s2
}

void BandAxis::fout(const char *tekst) {
  _stepper->forceStop();
  _gehomed = false;  // na een fout is de positie niet meer betrouwbaar
  _toestand = Toestand::ERROR;
  _fout = tekst;
  Serial.printf("!! FOUT: %s. Typ 'reset' om te wissen.\n", tekst);
}

float BandAxis::positieMm() const {
  return _stepper ? stappenNaarMm(_stepper->getCurrentPosition()) : 0.0f;
}

const char *BandAxis::toestandNaam() const {
  switch (_toestand) {
    case Toestand::IDLE:   return "IDLE";
    case Toestand::HOMING: return "HOMING";
    case Toestand::MOVING: return "MOVING";
    case Toestand::ERROR:  return "ERROR";
  }
  return "?";
}
