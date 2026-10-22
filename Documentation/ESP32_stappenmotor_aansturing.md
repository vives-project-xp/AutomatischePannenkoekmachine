# ESP32-stappenmotor aansturen met DRV8825

## 1. Doel van het project

In dit project sturen we een NEMA17-stappenmotor aan met een ESP32-WROOM-32 en een DRV8825-stappenmotordriver.

De ESP32 genereert pulsen waarmee de driver de motor laat draaien. Met een extra signaal bepalen we de draairichting.

**Gebruikte componenten:**

- ESP32-WROOM-32
- NEMA17-stappenmotor, 40 mm
- DRV8825-stappenmotordriver
- 24 V-voeding voor de stappenmotor
- LM2596-step-downmodule voor het omzetten van 24 V naar 5 V
- USB-kabel voor het programmeren van de ESP32
- Multimeter voor het controleren van de spanning

## 2. Software installeren

We gebruiken Visual Studio Code met de extensie PlatformIO IDE.

### Stap 1: Visual Studio Code

Download en installeer Visual Studio Code via:

[https://code.visualstudio.com/](https://code.visualstudio.com/)

### Stap 2: PlatformIO installeren

1. Open Visual Studio Code.
2. Open Extensions met `Ctrl + Shift + X`.
3. Zoek naar `PlatformIO IDE`.
4. Installeer de extensie.

### Stap 3: Een nieuw project maken

Maak een nieuw PlatformIO-project met de volgende instellingen:

| Instelling | Waarde |
|---|---|
| Name | `Stappenmotor-test-code` |
| Board | Espressif ESP32 Dev Module |
| Framework | Arduino |

PlatformIO maakt automatisch de nodige projectbestanden aan.

## 3. Aansluitingen

### ESP32 naar DRV8825

| ESP32 | DRV8825 | Functie |
|---|---|---|
| GPIO 18 | STEP | Bepaalt wanneer een stap wordt uitgevoerd |
| GPIO 19 | DIR | Bepaalt de draairichting |
| GND | GND, logische zijde | Gemeenschappelijke massa |
| 3,3 V | RST en SLP | Houdt de driver uit reset en uit slaapstand |

De pinnen RST en SLP zijn met elkaar verbonden en aangesloten op 3,3 V.

### Instelling van de microstepping

Voor de eerste test gebruiken we full-step-modus.

| DRV8825-pin | Aansluiting |
|---|---|
| M0 | GND |
| M1 | GND |
| M2 | GND |
| EN | GND of open laten bij een standaardmodule |

In full-step-modus voert de motor bij een staphoek van 1,8° theoretisch 200 stappen per volledige omwenteling uit.

### Aansluiting van de motor

De NEMA17-stappenmotor wordt aangesloten op de vier motoruitgangen van de DRV8825.

De twee draden van elke motorspoel moeten correct bij elkaar horen. De spoelparen moeten worden bepaald aan de hand van de motorspecificaties of metingen.

### Voeding van de driver

De DRV8825 krijgt zijn motorvoeding van de externe 24 V-voeding.

| Aansluiting | Verbinding |
|---|---|
| VMOT | Positieve pool van de 24 V-voeding |
| GND bij VMOT | Negatieve pool van de 24 V-voeding |

De logische GND van de driver wordt verbonden met de GND van de ESP32.

**Veiligheid:** schakel de voeding uit voordat je bedrading aansluit of wijzigt. Sluit de motor nooit aan of af terwijl de driver onder spanning staat.

## 4. Stroomlimiet instellen

De gebruikte NEMA17-stappenmotor heeft de volgende opgegeven specificaties:

| Eigenschap | Waarde |
|---|---|
| Staphoek | 1,8° |
| Nominale stroom | 1,2 A per fase |
| Nominale spanning | 2,55 V |
| Houdkoppel | 4,08 kg·cm |

Op de meetweerstand van de DRV8825 staat `R100`. Dit komt bij de gebruikelijke uitvoering overeen met een meetweerstand van 0,10 Ω.

Voor die uitvoering geldt:

$$I_{\text{limiet}} = 2 \times V_{\text{REF}}$$

De theoretische VREF-doelwaarde voor een stroomlimiet van 1,2 A is:

$$V_{\text{REF}} = \frac{1{,}2}{2} = 0{,}60\ \text{V}$$

De spanning wordt gemeten tussen VREF en GND met een multimeter op gelijkspanning (DC V).

De stroomlimiet moet zorgvuldig worden ingesteld. De toegestane stroom hangt ook af van de koeling en de exacte uitvoering van de driver.

## 5. Voeding van de ESP32

Tijdens het programmeren en de eerste tests voeden we de ESP32 via USB.

Voor gebruik met één gezamenlijke 24 V-voeding kan een LM2596-step-downmodule worden gebruikt om de spanning om te zetten naar 5 V.

Voor het aansluiten van de ESP32 moet de uitgang van de step-downmodule met een multimeter op 5,0 V worden ingesteld en gecontroleerd.

De 24 V-voeding mag nooit rechtstreeks op de ESP32 worden aangesloten.

## 6. PlatformIO configureren

Open `platformio.ini` en gebruik de volgende configuratie:

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
```

## 7. Programmacode

Open `src/main.cpp` en plaats de volgende code:

```cpp
#include <Arduino.h>

const int STEP_PIN = 18;
const int DIR_PIN = 19;

const int STAPPEN = 200;
const int STAP_VERTRAGING = 800;

void draaiMotor(bool richting) {
    digitalWrite(DIR_PIN, richting);

    delay(10);

    for (int i = 0; i < STAPPEN; i++) {
        digitalWrite(STEP_PIN, HIGH);
        delayMicroseconds(STAP_VERTRAGING);

        digitalWrite(STEP_PIN, LOW);
        delayMicroseconds(STAP_VERTRAGING);
    }
}

void setup() {
    Serial.begin(115200);

    pinMode(STEP_PIN, OUTPUT);
    pinMode(DIR_PIN, OUTPUT);

    digitalWrite(STEP_PIN, LOW);

    Serial.println("Stappenmotor gestart");
}

void loop() {
    Serial.println("Draaien in richting 1");
    draaiMotor(HIGH);

    delay(1000);

    Serial.println("Draaien in richting 2");
    draaiMotor(LOW);

    delay(1000);
}
```

## 8. De code uploaden

1. Sluit de ESP32 met een USB-kabel aan op de computer.
2. Open het PlatformIO-project in Visual Studio Code.
3. Klik op **Build** om de code te compileren.
4. Klik op **Upload** om de code naar de ESP32 te sturen.
5. Open de PlatformIO Serial Monitor.

De seriële monitor is ingesteld op 115200 baud.

## 9. Werking van het programma

De code gebruikt twee GPIO-pinnen.

- `STEP_PIN`: genereert de pulsen voor de stappenmotor.
- `DIR_PIN`: bepaalt de draairichting.

De functie `draaiMotor()` stelt de richting in en genereert vervolgens 200 stappen.

Met een motor van 1,8° per stap en full-step-instelling komt dit theoretisch overeen met één volledige omwenteling.

De vertraging van 800 microseconden bepaalt de tijd tussen de STEP-pulsen. Na iedere beweging wacht het programma één seconde voordat de richting wordt omgekeerd.

Dit proces wordt voortdurend herhaald.

## 10. Veiligheid

- Sluit nooit 24 V rechtstreeks aan op de ESP32.
- Wijzig geen bedrading terwijl de voeding ingeschakeld is.
- Sluit de motor niet aan of af terwijl de driver onder spanning staat.
- Controleer de VREF en de stroomlimiet voordat je langdurig test.
- Controleer de temperatuur van de driver tijdens gebruik.
