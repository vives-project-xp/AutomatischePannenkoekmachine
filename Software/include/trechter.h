/*
 * trechter.h
 *
 * Twee gelijk lopende stappenmotoren die de trechter bewegen. Beide krijgen
 * op hetzelfde moment dezelfde beweging. Cyclus: 3 s heen, 5 s wachten,
 * 3 s terug, 15 s wachten, en opnieuw. Stopt bij noodstop of een geraakte
 * eindschakelaar (S1 op GPIO16, S2 op GPIO17).
 */
#pragma once

#include <Arduino.h>
#include <FastAccelStepper.h>

class Trechter {
 public:
  bool begin(FastAccelStepperEngine &engine);
  void update();  // elke loop-doorgang aanroepen, blokkeert nooit

  void start();   // cyclus starten (drivers aan)
  void stop();    // rustig afremmen en cyclus stoppen
  const char *toestandNaam() const;

 private:
  enum class Fase : uint8_t { UIT, HEEN, WACHT_UIT, TERUG, WACHT_THUIS, FOUT };

  void beweeg(bool heen);
  bool staatStil() const;
  void noodStop(const char *reden);

  FastAccelStepper *_m1 = nullptr;
  FastAccelStepper *_m2 = nullptr;
  Fase _fase = Fase::UIT;
  uint32_t _sinds = 0;
};
