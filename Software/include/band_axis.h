/*
 * band_axis.h
 *
 * De as "BAND": twee NEMA17-motoren op een gedeelde STEP/DIR-lijn die samen
 * de brug over de rail bewegen. Voor de software is het een enkele as.
 *
 * De klasse werkt in millimeter, bewaakt de softwarelimieten, voert de
 * homing uit en houdt een toestandsmachine bij (IDLE, HOMING, MOVING, ERROR).
 * De spatel- en draaimotor kunnen later op dezelfde manier als eigen klasse
 * worden toegevoegd; ze delen dan de FastAccelStepperEngine en Safety.
 */
#pragma once

#include <Arduino.h>
#include <FastAccelStepper.h>

class BandAxis {
 public:
  enum class Toestand : uint8_t { IDLE, HOMING, MOVING, ERROR };

  bool begin(FastAccelStepperEngine &engine);
  void update();  // elke loop-doorgang aanroepen, blokkeert nooit

  // Commando's. Geven false terug (en melden waarom) als ze geweigerd worden.
  bool home();
  bool gaNaar(float mm);     // absolute positie
  bool verplaats(float mm);  // relatief, mag negatief
  bool zetSnelheid(float mmPerS);
  bool zetVersnelling(float mmPerS2);
  void stop();        // rustig afremmen
  void vrijgeven();   // meteen stoppen voor 'disable'; positie is daarna onbekend
  bool resetFout();

  // Toestand opvragen
  Toestand toestand() const { return _toestand; }
  const char *toestandNaam() const;
  const char *foutTekst() const { return _fout; }
  bool isGehomed() const { return _gehomed; }
  float positieMm() const;
  float snelheid() const { return _snelheid; }
  float versnelling() const { return _versnelling; }

 private:
  // Deelstappen van de homing
  enum class HomeFase : uint8_t {
    ZOEK_SNEL,      // naar S1 aan 10 mm/s
    STOP_NA_SNEL,   // wachten tot de motor stilstaat
    TERUGRIJDEN,    // 5 mm weg van S1
    ZOEK_TRAAG,     // opnieuw naar S1 aan 2 mm/s
    STOP_NA_TRAAG,  // wachten tot de motor stilstaat, dan nulpunt zetten
    NAAR_NUL,       // van het schakelpunt naar positie 0 rijden
    AFBREKEN        // 'stop' tijdens homing: uitbollen en naar IDLE
  };

  bool kanStarten();
  bool naarStappen(int32_t doel);
  bool startRelatief(float mm, float mmPerS);
  void zetProfiel(float mmPerS, float mmPerS2);
  void updateHoming();
  void fout(const char *tekst);

  FastAccelStepper *_stepper = nullptr;
  Toestand _toestand = Toestand::IDLE;
  HomeFase _homeFase = HomeFase::ZOEK_SNEL;
  bool _gehomed = false;
  bool _noodstopVerwerkt = false;
  float _snelheid = 0;
  float _versnelling = 0;
  const char *_fout = "";
};
