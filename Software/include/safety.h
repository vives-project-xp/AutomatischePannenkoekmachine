/*
 * safety.h
 *
 * Veiligheidslaag die door alle assen gedeeld wordt:
 *  - noodstop-detectie via de 24V-meting op GPIO35, met vergrendeling
 *  - ontdenderde eindschakelaars (voorlopig alleen S1)
 *  - de gezamenlijke EN-lijn van alle stappenmotordrivers
 *
 * Een as vraagt hier alleen de toestand op; de as beslist zelf wat ze met
 * een geraakte eindschakelaar doet (tijdens homing is dat immers gewenst).
 */
#pragma once

#include <Arduino.h>

namespace Safety {

void begin();   // pinnen instellen, drivers uit
void update();  // elke loop-doorgang aanroepen

bool s1Geraakt();         // ontdenderd; true = geraakt OF kabel los
bool spanningAanwezig();  // true = 24 V aanwezig (noodstop niet ingedrukt)

// Vergrendeling: wordt gezet zodra de 24 V wegvalt en blijft staan tot reset()
bool vergrendeld();
bool reset();  // geeft false als de 24 V nog ontbreekt

// Gezamenlijke EN-lijn. Inschakelen wordt geweigerd zolang vergrendeld.
bool zetDrivers(bool aan);
bool driversAan();

}  // namespace Safety
