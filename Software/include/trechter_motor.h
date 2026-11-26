/*
 * trechter_motor.h
 *
 * Motor 3 (STEP 27 / DIR 14) opent en sluit de trechter. Een keer start()
 * geeft: 0,5 s open draaien, 3 s wachten, 0,5 s terug dicht.
 * Wordt aangestuurd door de bandcyclus (band_home.cpp) op positie 1 en 2.
 * Niet-blokkerend: update() keert meteen terug.
 */
#pragma once

#include <Arduino.h>
#include <FastAccelStepper.h>

namespace TrechterMotor {

bool begin(FastAccelStepperEngine &engine);
void update();  // elke loop-doorgang aanroepen, blokkeert nooit

bool start();   // open, wachten, dicht; false als de beweging niet kon starten
void stop();    // meteen stoppen
bool isKlaar(); // beweging volledig af, trechter weer dicht
bool isBezig(); // beweging loopt nog

}  // namespace TrechterMotor
