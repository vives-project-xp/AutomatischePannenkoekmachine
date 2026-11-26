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
constexpr uint8_t PIN_STEP_BAND  = 18;  // STEP bandmotor 1
constexpr uint8_t PIN_DIR_BAND   = 19;  // DIR bandmotor 1
constexpr uint8_t PIN_STEP_BAND2 = 25;  // STEP bandmotor 2
constexpr uint8_t PIN_DIR_BAND2  = 26;  // DIR bandmotor 2
constexpr uint8_t PIN_STEP_M3    = 27;  // STEP motor 3 (trechter)
constexpr uint8_t PIN_DIR_M3     = 14;  // DIR motor 3 (trechter)
constexpr uint8_t PIN_STEP_M4    = 23;  // STEP motor 4 (spatel)
constexpr uint8_t PIN_DIR_M4     = 22;  // DIR motor 4 (spatel)
constexpr uint8_t PIN_SPATEL_HOME = 21; // eindschakelaar spatel (NC naar GND)
constexpr uint8_t PIN_SERVO      = 32;  // servo PWM (nog niet gebruikt, vast LAAG)
constexpr uint8_t PIN_AAN_UIT     = 33;  // AAN/UIT-schakelaar (naar GND, interne pull-up)
// true = schakelaar AAN trekt de pin naar GND (LAAG). Werkt hij omgekeerd: false.
constexpr bool AAN_UIT_AAN_IS_LAAG = true;
constexpr uint8_t PIN_EN_DRIVERS = 13;  // EN van ALLE drivers, actief LAAG
constexpr uint8_t PIN_S1_HOME    = 16;  // eindschakelaar S1 (NC naar GND)
constexpr uint8_t PIN_S2_HOME    = 17;  // eindschakelaar S2 (NC naar GND)
constexpr uint8_t PIN_NOODSTOP   = 35;  // 24V-detectie (input-only)

// Trechtermotoren (DRV8825, RESET en SLP op 3V3)
constexpr uint8_t PIN_STEP_TRECHTER1 = 18;
constexpr uint8_t PIN_DIR_TRECHTER1  = 19;
constexpr uint8_t PIN_STEP_TRECHTER2 = 25;
constexpr uint8_t PIN_DIR_TRECHTER2  = 26;

// Gereserveerd voor latere uitbreidingen (nog niet gebruikt):
//   I2C en de SSR bakplaat hebben nieuwe pinnen nodig (21-23 zijn in gebruik)
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
// Zelfde voor bandmotor 2. Draait die tegen motor 1 in, zet dit dan op false.
constexpr bool DIR2_HOOG_TELT_OP = true;

// ---------------------------------------------------------------------------
// Bewegingsprofiel
// ---------------------------------------------------------------------------
constexpr float MAX_SNELHEID_MM_S  = 5.0f;    // startwaarde na opstarten (laag voor de test)
constexpr float VERSNELLING_MM_S2  = 50.0f;   // startwaarde na opstarten

// Bovengrenzen voor de commando's 'speed' en 'accel' (tegen typfouten)
constexpr float SNELHEID_LIMIET_MM_S   = 200.0f;
constexpr float VERSNELLING_LIMIET_MM_S2 = 2000.0f;

// Motor 3 (trechter): op positie 1 en 2 open (0,5 s), 3 s wachten, dicht (0,5 s)
constexpr uint32_t TRECHTER_M_SNELHEID_HZ = 200;   // stappen/s
constexpr uint32_t TRECHTER_M_VERSNELLING = 4000;  // stappen/s2
constexpr uint32_t TRECHTER_M_DRAAI_MS    = 500;   // duur van openen en van sluiten
constexpr uint32_t TRECHTER_M_WACHT_MS    = 3000;  // trechter open laten
constexpr bool TRECHTER_M_RICHTING_OMGEKEERD = true;  // draairichting omkeren

// Motor 4 (spatel): omhoog draaien tot de eindschakelaar (GPIO21) raakt
constexpr uint32_t SPATEL_SNELHEID_HZ = 200;    // stappen/s
constexpr uint32_t SPATEL_VERSNELLING = 4000;   // stappen/s2
constexpr uint32_t SPATEL_HOME_MAX_MS = 20000;  // time-out als de schakelaar niet komt
// Draait de spatel omlaag i.p.v. omhoog naar de eindschakelaar? Zet dit om.
constexpr bool SPATEL_OMHOOG_DIR_HOOG = true;

// Test: alleen de spatel laten werken, motor 1, 2 en 3 blijven stil.
// Zet op false om de bandcyclus (met trechter) weer te gebruiken.
constexpr bool TEST_ALLEEN_SPATEL = true;

// ---------------------------------------------------------------------------
// Trechter: heen, wachten, terug, wachten, herhalen
// ---------------------------------------------------------------------------
constexpr uint32_t TRECHTER_SNELHEID_HZ = 800;   // stappen/s (half zo snel als eerst)
constexpr uint32_t TRECHTER_VERSNELLING = 4000;  // stappen/s2
constexpr uint32_t TRECHTER_LOOPTIJD_MS = 3000;  // heen en terug duren elk 3 s
constexpr uint32_t TRECHTER_WACHT_UIT_MS  = 5000;   // wachttijd op de verre positie
constexpr uint32_t TRECHTER_WACHT_THUIS_MS = 15000; // wachttijd op de beginpositie
// Draairichting heen. Beweegt de trechter eerst de verkeerde kant op: omdraaien.
constexpr bool TRECHTER_HEEN_HOOG = true;
// Motor 2 staat gespiegeld; zet op true als die tegen motor 1 in draait.
constexpr bool TRECHTER_MOTOR2_OMGEKEERD = false;

// ---------------------------------------------------------------------------
// Bandmotoren: naar de startpositie (rechts)
// ---------------------------------------------------------------------------
constexpr uint32_t BAND_HOME_SNELHEID_HZ = 500;   // stappen/s
constexpr uint32_t BAND_HOME_VERSNELLING = 4000;   // stappen/s2
constexpr uint32_t BAND_HOME_MAX_MS      = 60000;  // time-out als de schakelaar niet komt
// Cyclus na het homen: wachten, 2x een stuk weg van de schakelaar, daarna terug
constexpr uint32_t BAND_WACHT_START_MS = 3000;  // wachten op de startpositie
constexpr uint32_t BAND_LOOPTIJD_MS    = 1500;  // elke beweging weg duurt 1,5 s
constexpr uint32_t BAND_WACHT_UIT_MS   = 5000;  // wachten na elke beweging weg
constexpr uint32_t BAND_CYCLUS_MS     = 60000; // elke cyclus start om de minuut
// Elke motor heeft zijn eigen eindschakelaar rechts: motor 1 = S1 (GPIO16),
// motor 2 = S2 (GPIO17). Zit het andersom, zet dit dan op true.
constexpr bool BAND_SCHAKELAARS_GEWISSELD = false;
// Draaien de motoren naar links i.p.v. rechts: zet dit om.
constexpr bool BAND_RECHTS_DIR_HOOG = false;
// Motor 2 staat gespiegeld; zet op true als die tegen motor 1 in draait.
constexpr bool BAND_MOTOR2_OMGEKEERD = false;

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
// Testmodus (voor de bandmotoren op de werkbank)
// ---------------------------------------------------------------------------
// TEST_NOODSTOP_UIT: negeert de 24V-detectie op GPIO35 (nog niet aangesloten).
// TEST_AUTOSTART_RUN: drivers aan en beide bandmotoren draaien na het opstarten
// meteen continu vooruit, tot 'stop'. Zet beide op false voor normaal gebruik.
constexpr bool TEST_NOODSTOP_UIT   = true;
constexpr bool TEST_AUTOSTART_RUN  = true;

// ---------------------------------------------------------------------------
// Veiligheid en communicatie
// ---------------------------------------------------------------------------
constexpr uint32_t ONTDENDER_MS = 5;       // ontdendertijd eindschakelaars
constexpr uint32_t SERIEEL_BAUD = 115200;
