# De automatische pannenkoekmachine

Welkom! Dit is het groepsproject PX 3 van Elektronica-ICT aan VIVES Hogeschool.
We bouwen een machine die zelf pannenkoeken bakt. 
[afbeelding](Media/Poster/Poster_versie1.png)
 
---
 
## In het kort
 
De machine doet drie dingen, na elkaar:
 
1. **Beslag gieten**: een trechter laat een vaste hoeveelheid beslag op de bakplaat lopen.
2. **Omdraaien**: een spatel keert de pannenkoek als de eerste kant gaar is.
3. **Afnemen**: de spatel schuift de gebakken pannenkoek van de plaat en legt het in een bord.
 
---
 
## Hoe zit de machine in elkaar?
 

 
| Deel | Wat is het? |
|---|---|
| **Het frame** | Het skelet van de machine, gemaakt van aluminium profielen. |
| **De bewegende delen** | Steppermotoren die door wieltjes en rubbere banden de spatel aandrijven.|
| **De trechter** | Een doseertrechter met beslag, die boven de bakplaat hangt. |
 

---
 
## Aan de slag
 
### 1. Onderdelen printen
 
- **Materiaal:** gebruik **PETG**. Dat is sterker dan PLA en breekt minder snel.
- **Instellingen:** minstens 4 wanden (perimeters) en 30 tot 40 % opvulling (infill).
- **Hoe leg je het op het bed?** Leg de gaten en openingen naar boven. Zo heb je geen support nodig.

 
### 2. Monteren
 
- **Wieltjes:** druk een kogellager in elk wieltje. Bevestig ze met **M8-bouten** en een afstandsbusje.
- **Onderste wieltje:** dat zit in een langwerpig gat. Duw het omhoog tot alle wieltjes het profiel raken en draai het dan vast.
- **Profielen in de houders:** schuif het profiel in de opening, zodat de T-sleutels in de sleuven glijden.
- **Op hoogte zetten:** gebruik **M5-schroeven met T-moeren** in de gaten van de rechtopstaande houders.
### 3. Benodigdheden

| Wat | Waarvoor |
|---|---|
| Aluminium profiel 20×20 B-type sleuf 6 | Het frame |
| Binnenhoeken 20 B-type sleuf 6 | Profielen haaks verbinden |
| 608-2RS kogellagers | In de wieltjes |
| Stappenmotoren NEMA17 + DRV8825-drivers | De aandrijving (banden en spatel) |
| GT2-riem (9 mm en 6 mm) + pulleys | De beweging overbrengen |
| M8-bouten + borgmoeren + afstandsbusjes | De wieltjes |
| M5-schroeven + hamermoeren (sleuf 6) | Houders op het profiel |
| M3-schroeven | Motoren en riemklemmen |
| ESP32 | De machine aansturen |
| Voeding 24 V + step-down naar 5 V | Stroom voor motoren en ESP32 |
| Eindschakelaars | Weten waar de wagentjes staan |
| Noodstop | Alles meteen stilzetten |
| Rvs-beslagdispenser Ø 13 cm | De trechter |
| Pannenkoekenmaker 30 cm | De bakplaat |

De volledige lijst met aantallen en prijzen staat in [Bestellijst](Documentation\BOM-list\Bestellijst_pannenkoekmachine.xlsx).
---
 
## Waar zijn we nog mee bezig?
 
- [ ] Frame in elkaar steken.
- [ ] Spatel laten draaien en schuiven.
- [ ] elektrisch shema maken.
- [ ] De software schrijven voor de servo motoren.


## Ons team

Wij zijn studenten aan VIVES Hogeschool:

| Naam | Opleiding | Studiejaar |
| --- | --- | --- |
| Brent Viveyn | Game & Entertainment Technology | 2de jaar |
| Hengda Lin | Network & System Administration | 2de jaar |
| Lars Ysebaert | Game & Entertainment Technology | 2de jaar |
| Mauro Carlier | Game & Entertainment Technology | 3de jaar |

## Projectpagina's

De onderstaande pagina's worden tijdens het project aangevuld:

- [Projectplanning](docs/projectplanning.md)
- [Mechanisch ontwerp](docs/mechanisch-ontwerp.md)
- [Elektronica en PCB](docs/elektronica-en-pcb.md)
- [Software](docs/software.md)
- [Testen](docs/testen.md)

**Opleiding:** VIVES Hogeschool - Elektronica-ICT

**Product owner:** Pedro Calleeuw
