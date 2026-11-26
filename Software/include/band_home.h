/*
 * band_home.h
 *
 * De twee bandmotoren doen altijd precies hetzelfde. Cyclus:
 *  1. naar rechts draaien tot een eindschakelaar raakt (startpositie)
 *  2. 3 s wachten
 *  3. 1,5 s van de schakelaar af; trechter open/dicht, min. 5 s wachten
 *  4. nog eens 1,5 s van de schakelaar af; trechter open/dicht, min. 5 s wachten
 *  5. direct terug naar de startpositie (tot de eindschakelaar raakt) en stoppen
 */
#pragma once

#include <Arduino.h>
#include <FastAccelStepper.h>

class BandHome {
 public:
  enum class Fase : uint8_t {
    UIT, ZOEKT, WACHT_START, WEG_1, WACHT_1, WEG_2, WACHT_2, TERUG, KLAAR, FOUT
  };

  bool begin(FastAccelStepperEngine &engine);
  void update();  // elke loop-doorgang aanroepen, blokkeert nooit

  void start();   // cyclus starten (drivers aan, eerst naar de startpositie)
  void stop();    // meteen stoppen
  Fase fase() const { return _fase; }
  const char *faseNaam() const;

 private:
  void fout(const char *reden);
  void beweegWeg();
  bool staatStil() const;
  bool wachtKlaar();

  FastAccelStepper *_m1 = nullptr;
  FastAccelStepper *_m2 = nullptr;
  Fase _fase = Fase::UIT;
  uint32_t _sinds = 0;
  uint32_t _cyclusStart = 0;     // begin van de huidige cyclus (voor de minuut)
  bool _schakelaarVrij = false;  // schakelaar is losgekomen tijdens beweging weg
};
