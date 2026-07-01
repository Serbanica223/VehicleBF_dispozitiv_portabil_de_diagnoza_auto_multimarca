# VehicleBF

VehicleBF este un dispozitiv portabil de diagnoză auto, dezvoltat pe baza unui microcontroller ESP32-S3. Proiectul permite afișarea unor informații OBD-II pe un display TFT și oferă două direcții principale de comunicație:

* diagnoză multimarcă prin adaptor ELM327 Wi-Fi;
* comunicație CAN dedicată prin interfața TWAI a ESP32-S3 și transceiver SN65HVD230.

Proiectul a fost realizat în scop educațional, ca parte a unei lucrări de licență, și urmărește înțelegerea comunicației OBD-II/CAN, a interfețelor embedded și a realizării unui sistem portabil de diagnoză.

---

## Funcționalități

* Interfață grafică pe display TFT ILI9341;
* Navigare prin keypad 4x3;
* Conectare la adaptor ELM327 Wi-Fi;
* Citire PID-uri OBD-II suportate;
* Afișare date live:

  * turație motor;
  * viteză vehicul;
  * temperatură lichid răcire;
  * temperatură aer admisie;
  * poziție clapetă accelerație;
  * sarcină motor;
  * debit aer MAF;
  * nivel combustibil;
  * tensiune ECU;
* Citire coduri DTC;
* Ștergere coduri DTC;
* Citire VIN;
* Afișare protocol OBD;
* Reglare luminozitate display;
* Mod dedicat Renault Logan 2 prin CAN;
* Simulator CAN pentru testarea mesajelor specifice.

---

## Arhitectură generală

---

## Componente hardware

| Componentă           | Rol în proiect                                   |
| -------------------- | ------------------------------------------------ |
| ESP32-S3 DevKitC-1   | Unitatea principală de procesare                 |
| TFT ILI9341 2.8”     | Afișarea meniurilor și datelor de diagnoză       |
| Keypad 4x3           | Navigarea prin meniuri                           |
| ELM327 Wi-Fi         | Interfață OBD-II multimarcă                      |
| SN65HVD230           | Transceiver CAN 3.3 V                            |
| DB9 / conector CAN   | Conectarea la magistrala CAN                     |
| Baterie Li-ion 3.7 V | Alimentare portabilă                             |
| Modul power-bank 5 V | Ridicare tensiune și alimentare sistem           |
| LM2596S              | Conversie tensiune pentru alimentare stabilizată |

---

## Conexiuni principale

### Display TFT ILI9341

| Semnal   | GPIO ESP32-S3 |
| -------- | ------------- |
| TFT_CS   | GPIO10        |
| TFT_DC   | GPIO9         |
| TFT_RST  | GPIO14        |
| TFT_MOSI | GPIO11        |
| TFT_SCK  | GPIO12        |
| TFT_MISO | GPIO13        |
| TFT_BL   | GPIO4         |

### Keypad 4x3

| Linie keypad | GPIO ESP32-S3 |
| ------------ | ------------- |
| R1           | GPIO2         |
| R2           | GPIO42        |
| R3           | GPIO41        |
| R4           | GPIO40        |
| C1           | GPIO39        |
| C2           | GPIO38        |
| C3           | GPIO37        |

### Navigare keypad

| Tastă | Funcție                                                      |
| ----- | ------------------------------------------------------------ |
| 2     | Sus                                                          |
| 8     | Jos                                                          |
| 5     | OK / Selectare                                               |
| *     | Înapoi                                                       |
| 4     | Scădere luminozitate                                         |
| 6     | Creștere luminozitate                                        |
| #     | Pornire/Oprire hardware prin pinul K al modulului power-bank |

---

## Software

Proiectul este dezvoltat în Arduino IDE pentru placa ESP32-S3.

### Biblioteci utilizate

* `WiFi.h` – conectarea ESP32-S3 la adaptorul ELM327 Wi-Fi;
* `SPI.h` – comunicația SPI cu display-ul TFT;
* `Adafruit_GFX.h` – funcții grafice generale;
* `Adafruit_ILI9341.h` – controlul display-ului ILI9341;
* `Keypad.h` – citirea tastelor keypad-ului;
* `ELMduino.h` – comunicația cu adaptorul ELM327 și solicitarea PID-urilor OBD-II.

---

## Structura meniului

---
