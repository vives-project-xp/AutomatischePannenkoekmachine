/*
 * spatel_motor.h
 *
 * Motor 4 (STEP 23 / DIR 22) kantelt de spatel. De eindschakelaar op GPIO21
 * geeft de startpositie: spatel bijna volledig omhoog.
 * Een keer start(): omhoog draaien tot de eindschakelaar raakt en stoppen.
 * Niet-blokkerend: update() keert meteen terug.
 */
#pragma once

#include <Arduino.h>
#include <FastAccelStepper.h>

namespace SpatelMotor {

bool begin(FastAccelStepperEngine &engine);
void update();  // elke loop-doorgang aanroepen, blokkeert nooit

void start();   // omhoog tot de eindschakelaar
void stop();    // meteen stoppen
const char *faseNaam();

}  // namespace SpatelMotor
