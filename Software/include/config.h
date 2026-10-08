/*
 * config.h
 *
 * Alle pinnen en instelbare constanten van de pannenkoekmachine op een plek.
 * Wil je iets aan de hardware of de afstelling wijzigen (pin, snelheid,
 * slaglengte, draairichting), dan doe je dat hier en nergens anders.
 */
#pragma once

#include <Arduino.h>

// ---------------------------------------------------------------------------
// Pinnen
// ---------------------------------------------------------------------------
constexpr uint8_t PIN_STEP_BAND  = 25;  // STEP, gedeeld door beide banddrivers
constexpr uint8_t PIN_DIR_BAND   = 26;  // DIR, gedeeld door beide banddrivers
constexpr uint8_t PIN_EN_DRIVERS = 13;  // EN van ALLE drivers, actief LAAG
constexpr uint8_t PIN_S1_HOME    = 16;  // eindschakelaar S1 (NC naar GND)
constexpr uint8_t PIN_NOODSTOP   = 35;  // 24V-detectie (input-only)

// Gereserveerd voor latere uitbreidingen (nog niet gebruikt):
//   27 / 14  STEP / DIR spatelmotor
//   32 / 33  STEP / DIR draaimotor
//   17 / 18  extra eindschakelaars
//   21 / 22  I2C (SDA / SCL)
//   23       SSR bakplaat
//   34       NTC temperatuurmeting
// Niet gebruiken: 0, 2, 5, 12, 15 (strapping) en 6-11 (flash).

// ---------------------------------------------------------------------------
// Mechanica van de band
// ---------------------------------------------------------------------------
// 200 stappen/omw * 16 microstappen = 3200 stappen/omw
// GT2-riem, 20 tanden * 2 mm = 40 mm/omw  ->  3200 / 40 = 80 stappen/mm
constexpr float STEPS_PER_MM = 80.0f;

constexpr float SLAGLENGTE_MM = 600.0f;  // bruikbare slag (nog na te meten)

// Draairichting: true = DIR hoog laat de positie oplopen (weg van S1).
// Rijdt de brug bij 'home' WEG van S1, zet dit dan op false.
constexpr bool DIR_HOOG_TELT_OP = true;

// ---------------------------------------------------------------------------
// Bewegingsprofiel
// ---------------------------------------------------------------------------
constexpr float MAX_SNELHEID_MM_S  = 50.0f;   // startwaarde na opstarten
constexpr float VERSNELLING_MM_S2  = 200.0f;  // startwaarde na opstarten

// Bovengrenzen voor de commando's 'speed' en 'accel' (tegen typfouten)
constexpr float SNELHEID_LIMIET_MM_S   = 200.0f;
constexpr float VERSNELLING_LIMIET_MM_S2 = 2000.0f;

// ---------------------------------------------------------------------------
// Homing
// ---------------------------------------------------------------------------
constexpr float HOME_SNELHEID_SNEL_MM_S  = 10.0f;  // eerste benadering
constexpr float HOME_SNELHEID_TRAAG_MM_S = 2.0f;   // tweede, nauwkeurige benadering
constexpr float HOME_TERUGRIJ_MM         = 5.0f;   // terugrijden na eerste contact
constexpr float HOME_EXTRA_ZOEK_MM       = 50.0f;  // marge bovenop de slaglengte

// Afstand tussen het schakelpunt van S1 en positie 0. Zo is S1 op positie 0
// weer vrij en geeft 'goto 0' geen valse eindschakelaarfout.
constexpr float HOME_OFFSET_MM = 3.0f;

// ---------------------------------------------------------------------------
// Veiligheid en communicatie
// ---------------------------------------------------------------------------
constexpr uint32_t ONTDENDER_MS = 5;       // ontdendertijd eindschakelaars
constexpr uint32_t SERIEEL_BAUD = 115200;
