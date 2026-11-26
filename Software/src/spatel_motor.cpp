/*
 * spatel_motor.cpp
 *
 * Spatelmotor: draait omhoog (runForward) tot de eindschakelaar op GPIO21
 * raakt en stopt dan meteen. Dat is de startpositie.
 */
#include "spatel_motor.h"

#include "config.h"
#include "safety.h"

namespace {

enum class Fase : uint8_t { UIT, OMHOOG, KLAAR, FOUT };

FastAccelStepper *motor = nullptr;
Fase fase = Fase::UIT;
uint32_t sinds = 0;

}  // namespace

namespace SpatelMotor {

bool begin(FastAccelStepperEngine &engine) {
  motor = engine.stepperConnectToPin(PIN_STEP_M4);
  if (motor == nullptr) return false;
  // runForward() = omhoog (naar de eindschakelaar)
  motor->setDirectionPin(PIN_DIR_M4, SPATEL_OMHOOG_DIR_HOOG);
  motor->setSpeedInHz(SPATEL_SNELHEID_HZ);
  motor->setAcceleration(SPATEL_VERSNELLING);
  // EN is gedeeld en wordt door Safety beheerd: bewust geen setEnablePin().
  return true;
}

void start() {
  if (motor == nullptr) return;
  if (Safety::spatelGeraakt()) {
    fase = Fase::KLAAR;
    Serial.println(F("Spatel: eindschakelaar al ingedrukt, staat op startpositie"));
    return;
  }
  Serial.println(F("Spatel: omhoog naar de eindschakelaar..."));
  motor->runForward();
  fase = Fase::OMHOOG;
  sinds = millis();
}

void stop() {
  if (motor == nullptr) return;
  motor->forceStop();
  fase = Fase::UIT;
}

void update() {
  if (motor == nullptr || fase != Fase::OMHOOG) return;

  if (Safety::vergrendeld()) {
    motor->forceStop();
    fase = Fase::FOUT;
    Serial.println(F("!! Spatel gestopt: noodstop"));
  } else if (Safety::spatelGeraakt()) {
    motor->forceStop();
    motor->setCurrentPosition(0);
    fase = Fase::KLAAR;
    Serial.printf("Spatel: eindschakelaar bereikt na %lu ms, gestopt\n",
                  (unsigned long)(millis() - sinds));
  } else if (millis() - sinds > SPATEL_HOME_MAX_MS) {
    motor->forceStop();
    fase = Fase::FOUT;
    Serial.println(F("!! Spatel gestopt: eindschakelaar niet bereikt binnen de maximale tijd"));
  }
}

const char *faseNaam() {
  switch (fase) {
    case Fase::UIT:    return "UIT";
    case Fase::OMHOOG: return "OMHOOG";
    case Fase::KLAAR:  return "OP STARTPOSITIE";
    case Fase::FOUT:   return "FOUT";
  }
  return "?";
}

}  // namespace SpatelMotor
