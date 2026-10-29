/*
 * spatel_motor.h
 *
 * Derde stappenmotor (STEP 27 / DIR 14). Voorlopig een testpatroon: de motor
 * draait continu en wisselt elke 2 s van richting.
 * De klasse is een niet-blokkerende toestandsmachine; update() keert meteen terug.
 */
#pragma once

#include <Arduino.h>
#include <FastAccelStepper.h>

class SpatelMotor {
 public:
  bool begin(FastAccelStepperEngine &engine);
  void update();  // elke loop-doorgang aanroepen, blokkeert nooit

 private:
  enum class Fase : uint8_t { WACHT_OP_START, HEEN, TERUG };

  void naarFase(Fase fase);

  FastAccelStepper *_stepper = nullptr;
  Fase _fase = Fase::WACHT_OP_START;
  uint32_t _faseStart = 0;
};
