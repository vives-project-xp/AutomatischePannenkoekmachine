/*
 * trechter_motor.cpp
 *
 * Trechtermotor: TRECHTER_M_DRAAI_MS open draaien, TRECHTER_M_WACHT_MS
 * wachten en TRECHTER_M_DRAAI_MS terug. Heen en terug zijn exact hetzelfde
 * aantal stappen, zodat de motor precies op zijn beginpositie terugkomt.
 */
#include "trechter_motor.h"

#include "config.h"
#include "safety.h"

namespace {

enum class Fase : uint8_t { UIT, OPEN, WACHT, DICHT, KLAAR };

FastAccelStepper *motor = nullptr;
Fase fase = Fase::UIT;
uint32_t sinds = 0;

// Aantal stappen zodat een beweging TRECHTER_M_DRAAI_MS duurt
// (trapeziumprofiel: afstand = v * (T - v/a))
int32_t stappen() {
  float t = TRECHTER_M_DRAAI_MS / 1000.0f;
  float rampTijd = (float)TRECHTER_M_SNELHEID_HZ / TRECHTER_M_VERSNELLING;
  return lroundf(TRECHTER_M_SNELHEID_HZ * (t - rampTijd));
}

bool beweeg(int32_t aantal) {
  if (motor->move(aantal) != MOVE_OK) {
    Serial.println(F("!! Trechter: move mislukt"));
    motor->forceStop();
    fase = Fase::UIT;
    return false;
  }
  return true;
}

}  // namespace

namespace TrechterMotor {

bool begin(FastAccelStepperEngine &engine) {
  motor = engine.stepperConnectToPin(PIN_STEP_M3);
  if (motor == nullptr) return false;
  // Tweede argument: true = DIR hoog is vooruit. Omkeren draait de richting om.
  motor->setDirectionPin(PIN_DIR_M3, !TRECHTER_M_RICHTING_OMGEKEERD);
  motor->setSpeedInHz(TRECHTER_M_SNELHEID_HZ);
  motor->setAcceleration(TRECHTER_M_VERSNELLING);
  // EN is gedeeld en wordt door Safety beheerd: bewust geen setEnablePin().
  return true;
}

bool start() {
  if (motor == nullptr) return false;
  if (!beweeg(stappen())) return false;
  fase = Fase::OPEN;
  Serial.println(F("Trechter: open"));
  return true;
}

void stop() {
  if (motor == nullptr) return;
  motor->forceStop();
  fase = Fase::UIT;
}

bool isKlaar() { return fase == Fase::KLAAR; }

bool isBezig() { return fase == Fase::OPEN || fase == Fase::WACHT || fase == Fase::DICHT; }

void update() {
  if (motor == nullptr || !isBezig()) return;

  if (Safety::vergrendeld()) {
    stop();
    Serial.println(F("!! Trechter gestopt: noodstop"));
    return;
  }

  switch (fase) {
    case Fase::OPEN:
      if (!motor->isRunning()) {
        fase = Fase::WACHT;
        sinds = millis();
      }
      break;
    case Fase::WACHT:
      if (millis() - sinds >= TRECHTER_M_WACHT_MS) {
        if (beweeg(-stappen())) {
          fase = Fase::DICHT;
          Serial.println(F("Trechter: dicht"));
        }
      }
      break;
    case Fase::DICHT:
      if (!motor->isRunning()) fase = Fase::KLAAR;
      break;
    default:
      break;
  }
}

}  // namespace TrechterMotor
