/*
 * spatel_motor.cpp
 *
 * Testpatroon voor de derde motor: continu draaien en elke SPATEL_RICHTING_MS
 * van richting wisselen.
 * Het patroon loopt alleen als de drivers aan staan en er geen noodstop is.
 */
#include "spatel_motor.h"

#include "config.h"
#include "safety.h"

bool SpatelMotor::begin(FastAccelStepperEngine &engine) {
  _stepper = engine.stepperConnectToPin(PIN_STEP_SPATEL);
  if (_stepper == nullptr) return false;
  _stepper->setDirectionPin(PIN_DIR_SPATEL);
  // De EN-lijn is gedeeld en wordt door Safety beheerd
  _stepper->setSpeedInHz(SPATEL_SNELHEID_HZ);
  _stepper->setAcceleration(SPATEL_VERSNELLING);
  return true;
}

void SpatelMotor::naarFase(Fase fase) {
  _fase = fase;
  _faseStart = millis();
  switch (fase) {
    case Fase::HEEN:  _stepper->runForward();  break;
    case Fase::TERUG: _stepper->runBackward(); break;
    case Fase::WACHT_OP_START: break;
  }
}

void SpatelMotor::update() {
  if (_stepper == nullptr) return;

  // Zonder aan staande drivers of bij noodstop: meteen stoppen en opnieuw beginnen
  if (Safety::vergrendeld() || !Safety::driversAan()) {
    if (_fase != Fase::WACHT_OP_START) {
      _stepper->forceStop();
      _fase = Fase::WACHT_OP_START;
    }
    return;
  }

  uint32_t verstreken = millis() - _faseStart;
  switch (_fase) {
    case Fase::WACHT_OP_START:
      naarFase(Fase::HEEN);
      break;
    case Fase::HEEN:
      if (verstreken >= SPATEL_RICHTING_MS) naarFase(Fase::TERUG);
      break;
    case Fase::TERUG:
      if (verstreken >= SPATEL_RICHTING_MS) naarFase(Fase::HEEN);
      break;
  }
}
