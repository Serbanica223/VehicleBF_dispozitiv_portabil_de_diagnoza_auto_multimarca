#include <WiFi.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Keypad.h>
#include "ELMduino.h"
#include "driver/twai.h"


// ================= TFT =================
#define TFT_CS    10
#define TFT_DC    9
#define TFT_RST   14
#define TFT_MOSI  11
#define TFT_SCK   12
#define TFT_MISO  13
#define TFT_BL    4

#define RGB_LED   48

//la trans can 17 - TX , 18 - RX


#define CAN_TX_PIN GPIO_NUM_17   // ESP32-S3 -> D la SN65HVD230
#define CAN_RX_PIN GPIO_NUM_18   // ESP32-S3 <- R de la SN65HVD230

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

// ================= KEYPAD =================
const byte ROWS = 4;
const byte COLS = 3;

char keys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};

byte rowPins[ROWS] = {37, 38, 39, 40};
byte colPins[COLS] = {41, 42, 2};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

#define KEY_UP      '6'
#define KEY_DOWN    '9'
#define KEY_ENTER   '5'
#define KEY_BACK    '8'
#define KEY_SLEEP   '3'

// ================= BACKLIGHT =================
int brightnessLevel = 5;
const int brightnessValues[] = {25, 50, 80, 110, 140, 170, 200, 230, 255};
const int brightnessCount = sizeof(brightnessValues) / sizeof(brightnessValues[0]);

// ================= ELM327 Wi-Fi =================
// Schimba datele in functie de adaptorul tau ELM327 Wi-Fi.
const char* ELM_SSID = "WiFi_OBDII";
const char* ELM_PASS = "";

IPAddress ELM_IP(192, 168, 0, 10);
const uint16_t ELM_PORT = 35000;

WiFiClient elmClient;
ELM327 myELM327;

uint32_t supportedMask_1_20 = 0;
uint32_t supportedMask_21_40 = 0;
uint32_t supportedMask_41_60 = 0;
uint32_t supportedMask_61_80 = 0;

bool elmConnected = false;

// ================= MENIU =================
enum ScreenState {
  SCREEN_MAIN,
  SCREEN_MULTIBRAND,
  SCREEN_RENAULT,
  SCREEN_SETTINGS,
  SCREEN_INFO,
  SCREEN_ELM_DIAG,
  SCREEN_LIVE_DATA,
  SCREEN_DTC,
  SCREEN_CLEAR_DTC_CONFIRM,
  SCREEN_VIN,
  SCREEN_PROTOCOL,
  SCREEN_SUPPORTED_PIDS,
  SCREEN_BRIGHTNESS,
  SCREEN_RENAULT_CAN
};

ScreenState currentScreen = SCREEN_MAIN;
int selectedIndex = 0;

const char* mainMenu[] = {
  "Mod Multimarca",
  "Mod dedicat Logan 2",
  "Setari",
  "Info sistem"
};

const char* multiMenuDisconnected[] = {
  "Conectare ELM327 Wi-Fi",
  "Inapoi"
};

const char* multiMenuConnected[] = {
  "Diagnoza ELM327",
  "Reconectare ELM327",
  "Deconectare ELM327",
  "Inapoi"
};

const char* elmDiagMenu[] = {
  "Date live disponibile",
  "Citire DTC",
  "Stergere DTC",
  "VIN vehicul",
  "Protocol OBD",
  "PID-uri suportate",
  "Inapoi"
};

const char* renaultMenu[] = {
  "Diagnoza Renault",
  "Parametri Renault",
  "Inapoi"
};

const char* settingsMenu[] = {
  "Luminozitate display",
  "Inapoi"
};

// ================= DATE LIVE =================
struct LivePID {
  uint8_t pid;
  const char* label;
  const char* unit;
  float value;
  bool supported;
  bool hasValue;
  uint8_t decimals;
};

LivePID livePIDs[] = {
  {0x01, "Mon DTC",    "raw",  0, false, false, 0},
  {0x02, "Freeze DTC", "raw",  0, false, false, 0},
  {0x03, "Fuel stat",  "raw",  0, false, false, 0},

  {0x04, "Load",       "%",    0, false, false, 1},
  {0x05, "Temp apa",   "C",    0, false, false, 0},
  {0x06, "STFT B1",    "%",    0, false, false, 1},
  {0x07, "LTFT B1",    "%",    0, false, false, 1},
  {0x08, "STFT B2",    "%",    0, false, false, 1},
  {0x09, "LTFT B2",    "%",    0, false, false, 1},
  {0x0A, "Pres comb",  "kPa",  0, false, false, 0},
  {0x0B, "MAP",        "kPa",  0, false, false, 0},
  {0x0C, "RPM",        "rpm",  0, false, false, 0},
  {0x0D, "Viteza",     "km/h", 0, false, false, 0},
  {0x0E, "Avans",      "deg",  0, false, false, 1},
  {0x0F, "Temp adm",   "C",    0, false, false, 0},
  {0x10, "MAF",        "g/s",  0, false, false, 1},
  {0x11, "Clapeta",    "%",    0, false, false, 1},
  {0x12, "Aer sec",    "raw",  0, false, false, 0},
  {0x13, "O2 2 banc",  "raw",  0, false, false, 0},

  {0x1C, "OBD std",    "raw",  0, false, false, 0},
  {0x1D, "O2 4 banc",  "raw",  0, false, false, 0},
  {0x1E, "Aux input",  "raw",  0, false, false, 0},
  {0x1F, "Run time",   "s",    0, false, false, 0},

  {0x21, "Dist MIL",   "km",   0, false, false, 0},
  {0x22, "Rail pres",  "kPa",  0, false, false, 0},
  {0x23, "Rail gauge", "kPa",  0, false, false, 0},
  {0x2C, "EGR cmd",    "%",    0, false, false, 1},
  {0x2D, "EGR error",  "%",    0, false, false, 1},
  {0x2E, "Purge evap", "%",    0, false, false, 1},
  {0x2F, "Combust.",   "%",    0, false, false, 1},
  {0x30, "Warmups",    "cnt",  0, false, false, 0},
  {0x31, "Dist clear", "km",   0, false, false, 0},
  {0x32, "Evap pres",  "Pa",   0, false, false, 0},
  {0x33, "Baro pres",  "kPa",  0, false, false, 0},
  {0x3C, "Cat B1S1",   "C",    0, false, false, 1},
  {0x3D, "Cat B2S1",   "C",    0, false, false, 1},
  {0x3E, "Cat B1S2",   "C",    0, false, false, 1},
  {0x3F, "Cat B2S2",   "C",    0, false, false, 1},

  {0x41, "Mon drive",  "raw",  0, false, false, 0},
  {0x42, "Tens ECU",   "V",    0, false, false, 2},
  {0x43, "Load abs",   "%",    0, false, false, 1},
  {0x44, "AFR cmd",    "ratio",0, false, false, 3},
  {0x45, "Clap rel",   "%",    0, false, false, 1},
  {0x46, "Temp amb",   "C",    0, false, false, 0},
  {0x47, "Clap B",     "%",    0, false, false, 1},
  {0x48, "Clap C",     "%",    0, false, false, 1},
  {0x49, "Clap D",     "%",    0, false, false, 1},
  {0x4A, "Clap E",     "%",    0, false, false, 1},
  {0x4B, "Clap F",     "%",    0, false, false, 1},
  {0x4C, "Act clap",   "%",    0, false, false, 1},
  {0x4D, "MIL timp",   "min",  0, false, false, 0},
  {0x4E, "Clear timp", "min",  0, false, false, 0},
  {0x50, "MAF max",    "g/s",  0, false, false, 1},
  {0x51, "Tip comb",   "raw",  0, false, false, 0},
  {0x52, "Etanol",     "%",    0, false, false, 1},
  {0x53, "Evap abs",   "kPa",  0, false, false, 1},
  {0x54, "Evap pres2", "Pa",   0, false, false, 0},
  {0x59, "Rail abs",   "kPa",  0, false, false, 0},
  {0x5A, "Pedala",     "%",    0, false, false, 1},
  {0x5B, "Hybrid bat", "%",    0, false, false, 1},
  {0x5C, "Temp ulei",  "C",    0, false, false, 0},
  {0x5D, "Inject tim", "deg",  0, false, false, 1},
  {0x5E, "Fuel rate",  "L/h",  0, false, false, 1},
  {0x5F, "Emisii",     "raw",  0, false, false, 0},

  {0x61, "Torque req", "%",    0, false, false, 1},
  {0x62, "Torque act", "%",    0, false, false, 1},
  {0x63, "Torque ref", "Nm",   0, false, false, 0},
  {0x65, "Aux sup",    "raw",  0, false, false, 0}
};

const int LIVE_PID_COUNT = sizeof(livePIDs) / sizeof(livePIDs[0]);

int currentLivePID = 0;
unsigned long lastLiveDraw = 0;


// ================= DISPLAY =================
void drawHeader(const char* title) {
  tft.fillScreen(ILI9341_BLACK);
  tft.fillRect(0, 0, 320, 32, ILI9341_DARKGREY);

  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.setCursor(8, 8);
  tft.print(title);
}

void drawFooter(const char* text = "2=Sus  8=Jos  5=OK  *=Inapoi") {
  tft.fillRect(0, 220, 320, 20, ILI9341_BLACK);
  tft.setTextSize(1);
  tft.setTextColor(ILI9341_YELLOW);
  tft.setCursor(10, 225);
  tft.print(text);
}

void drawMenu(const char* title, const char** items, int count) {
  drawHeader(title);

  tft.setTextSize(2);

  for (int i = 0; i < count; i++) {
    int y = 50 + i * 30;

    if (i == selectedIndex) {
      tft.fillRect(5, y - 5, 310, 26, ILI9341_BLUE);
      tft.setTextColor(ILI9341_WHITE);
    } else {
      tft.setTextColor(ILI9341_LIGHTGREY);
    }

    tft.setCursor(12, y);
    tft.print(items[i]);
  }

  drawFooter();
}

void drawMessage(const char* title, const String& msg) {
  drawHeader(title);

  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE);

  int start = 0;
  int y = 55;

  while (start < msg.length()) {
    String line = msg.substring(start, start + 24);
    tft.setCursor(10, y);
    tft.print(line);
    y += 25;
    start += 24;

    if (y > 195) break;
  }

  drawFooter("*=Inapoi");
}

void drawStatus(const char* title, const String& msg) {
  drawHeader(title);
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(10, 65);
  tft.println(msg);
}

// ================= LUMINOZITATE =================
void applyBrightness() {
  int pwmValue = brightnessValues[brightnessLevel];
  analogWrite(TFT_BL, pwmValue);
}

void drawBrightnessScreen() {
  drawHeader("Luminozitate");

  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE);

  tft.setCursor(10, 60);
  tft.print("Nivel: ");
  tft.print(brightnessLevel + 1);
  tft.print("/");
  tft.print(brightnessCount);

  int barX = 20;
  int barY = 110;
  int barW = 280;
  int barH = 25;

  tft.drawRect(barX, barY, barW, barH, ILI9341_WHITE);

  int fillW = map(brightnessLevel + 1, 1, brightnessCount, 10, barW - 4);
  tft.fillRect(barX + 2, barY + 2, fillW, barH - 4, ILI9341_YELLOW);

  drawFooter("2=Creste  8=Scade  *=Inapoi");
}



//////////////////////////////////////////////////////////////////////////////////////////////CAN/////////////////////////////////
bool canStarted = false;

bool initCAN500k() {
  twai_general_config_t g_config =
    TWAI_GENERAL_CONFIG_DEFAULT(CAN_TX_PIN, CAN_RX_PIN, TWAI_MODE_NORMAL);

  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();

  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  if (twai_driver_install(&g_config, &t_config, &f_config) != ESP_OK) {
    return false;
  }

  if (twai_start() != ESP_OK) {
    twai_driver_uninstall();
    return false;
  }

  canStarted = true;
  return true;
}


struct RenaultCANData {
  int rpm = 0;
  int speed = 0;
  int coolantTemp = 0;
  bool engineOn = false;

  bool rpmValid = false;
  bool speedValid = false;
  bool tempValid = false;
  bool engineValid = false;

  unsigned long lastFrameTime = 0;
};

RenaultCANData renaultCAN;

void readCANMessages() {
  if (!canStarted) return;

  twai_message_t msg;

  while (twai_receive(&msg, 0) == ESP_OK) {
    if (msg.flags & TWAI_MSG_FLAG_EXTD) {
      continue;
    }

    renaultCAN.lastFrameTime = millis();

    switch (msg.identifier) {
      case 0x186: { // RPM motor
        if (msg.data_length_code >= 2) {
          uint16_t raw = ((uint16_t)msg.data[0] << 8) | msg.data[1];
          renaultCAN.rpm = raw / 10;
          renaultCAN.rpmValid = true;
        }
        break;
      }

      case 0x354: { // Viteza vehicul
        if (msg.data_length_code >= 2) {
          uint16_t raw = ((uint16_t)msg.data[0] << 8) | msg.data[1];
          renaultCAN.speed = raw / 100;
          renaultCAN.speedValid = true;
        }
        break;
      }

      case 0x5DA: { // Temperatura motor
        if (msg.data_length_code >= 1) {
          renaultCAN.coolantTemp = msg.data[0];
          renaultCAN.tempValid = true;
        }
        break;
      }

      case 0x1F6: { // Motor pornit
        if (msg.data_length_code >= 2) {
          renaultCAN.engineOn = (msg.data[1] & 0x20);
          renaultCAN.engineValid = true;
        }
        break;
      }

      default:
        break;
    }
  }
}

void drawRenaultCANScreen() {
  drawHeader("CAN Logan 2");

  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE);

  int y = 50;

  tft.setCursor(10, y);
  tft.print("RPM: ");
  if (renaultCAN.rpmValid) {
    tft.print(renaultCAN.rpm);
    tft.print(" rpm");
  } else {
    tft.print("...");
  }

  y += 30;

  tft.setCursor(10, y);
  tft.print("Viteza: ");
  if (renaultCAN.speedValid) {
    tft.print(renaultCAN.speed);
    tft.print(" km/h");
  } else {
    tft.print("...");
  }

  y += 30;

  tft.setCursor(10, y);
  tft.print("Temp: ");
  if (renaultCAN.tempValid) {
    tft.print(renaultCAN.coolantTemp);
    tft.print(" C");
  } else {
    tft.print("...");
  }

  y += 30;

  tft.setCursor(10, y);
  tft.print("Motor: ");
  if (renaultCAN.engineValid) {
    tft.print(renaultCAN.engineOn ? "Pornit" : "Oprit");
  } else {
    tft.print("...");
  }

  y += 30;

  tft.setCursor(10, y);
  tft.print("CAN: ");
  if (millis() - renaultCAN.lastFrameTime < 1000) {
    tft.print("Activ");
  } else {
    tft.print("Fara semnal");
  }

  drawFooter("*=Inapoi");
}

//////////////////////////////////////////////////////////////////////////////////////////////CAN/////////////////////////////////





// ================= ELMDUINO =================
bool checkELMConnection() {
  if (!elmConnected) return false;

  if (WiFi.status() != WL_CONNECTED) {
    elmConnected = false;
    return false;
  }

  if (!elmClient.connected()) {
    elmConnected = false;
    return false;
  }

  return true;
}

void disconnectELM327() {
  elmClient.stop();
  WiFi.disconnect(true);
  elmConnected = false;

  for (int i = 0; i < LIVE_PID_COUNT; i++) {
    livePIDs[i].supported = false;
    livePIDs[i].hasValue = false;
    livePIDs[i].value = 0;
  }
}

uint32_t readSupportedPIDsBlock(uint8_t block) {
  uint32_t value = 0;
  unsigned long start = millis();

  while (millis() - start < 5000) {
    if (block == 1) {
      value = myELM327.supportedPIDs_1_20();
    } 
    else if (block == 2) {
      value = myELM327.supportedPIDs_21_40();
    } 
    else if (block == 3) {
      value = myELM327.supportedPIDs_41_60();
    }
    else if (block == 4) {
      value = myELM327.supportedPIDs_61_80();
    }

    if (myELM327.nb_rx_state == ELM_SUCCESS) {
      return value;
    }

    if (myELM327.nb_rx_state != ELM_GETTING_MSG) {
      return 0;
    }

    delay(10);
  }

  return 0;
}

bool isPIDSupportedFromMask(uint8_t pid) {
  if (pid >= 0x01 && pid <= 0x20) {
    return supportedMask_1_20 & (1UL << (32 - pid));
  }

  if (pid >= 0x21 && pid <= 0x40) {
    return supportedMask_21_40 & (1UL << (32 - (pid - 0x20)));
  }

  if (pid >= 0x41 && pid <= 0x60) {
    return supportedMask_41_60 & (1UL << (32 - (pid - 0x40)));
  }

  if (pid >= 0x61 && pid <= 0x80) {
    return supportedMask_61_80 & (1UL << (32 - (pid - 0x60)));
  }

  return false;
}

void detectSupportedPIDs() {
  supportedMask_1_20 = 0;
  supportedMask_21_40 = 0;
  supportedMask_41_60 = 0;
  supportedMask_61_80 = 0;

  drawStatus("ELM327", "Citire PID 01-20...");
  supportedMask_1_20 = readSupportedPIDsBlock(1);

  if (supportedMask_1_20 != 0 && isPIDSupportedFromMask(0x20)) {
    drawStatus("ELM327", "Citire PID 21-40...");
    supportedMask_21_40 = readSupportedPIDsBlock(2);
  }

  if (supportedMask_21_40 != 0 && isPIDSupportedFromMask(0x40)) {
    drawStatus("ELM327", "Citire PID 41-60...");
    supportedMask_41_60 = readSupportedPIDsBlock(3);
  }

  if (supportedMask_41_60 != 0 && isPIDSupportedFromMask(0x60)) {
    drawStatus("ELM327", "Citire PID 61-80...");
    supportedMask_61_80 = readSupportedPIDsBlock(4);
  }

  for (int i = 0; i < LIVE_PID_COUNT; i++) {
    livePIDs[i].supported = isPIDSupportedFromMask(livePIDs[i].pid);
    livePIDs[i].hasValue = false;
    livePIDs[i].value = 0;
  }
}

bool connectToELM327() {
  disconnectELM327();

  drawStatus("ELM327", "Conectare Wi-Fi...");

  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.begin(ELM_SSID, ELM_PASS);

  unsigned long start = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - start < 12000) {
    delay(300);
  }

  if (WiFi.status() != WL_CONNECTED) {
    drawStatus("ELM327", "Wi-Fi esuat");
    delay(1600);
    return false;
  }

  drawStatus("ELM327", "Conectare TCP...");

  if (!elmClient.connect(ELM_IP, ELM_PORT)) {
    drawStatus("ELM327", "TCP esuat");
    delay(1600);
    return false;
  }

  drawStatus("ELM327", "Initializare...");

  bool elmOK = myELM327.begin(elmClient, false, 2000);

  if (!elmOK) {
    drawStatus("ELM327", "Init ELM esuat");
    delay(1600);
    elmClient.stop();
    WiFi.disconnect(true);
    return false;
  }

  elmConnected = true;

 drawStatus("ELM327", "Verificare PID-uri...");
 detectSupportedPIDs();

  currentLivePID = 0;

  drawStatus("ELM327", "Conexiune reusita");
  delay(1200);

  return true;
}

// ================= LIVE DATA CU ELMDUINO =================
float queryLivePIDByIndex(int index) {
  switch (index) {
    case 0:  return myELM327.monitorStatus();
    case 1:  return myELM327.freezeDTC();
    case 2:  return myELM327.fuelSystemStatus();

    case 3:  return myELM327.engineLoad();
    case 4:  return myELM327.engineCoolantTemp();
    case 5:  return myELM327.shortTermFuelTrimBank_1();
    case 6:  return myELM327.longTermFuelTrimBank_1();
    case 7:  return myELM327.shortTermFuelTrimBank_2();
    case 8:  return myELM327.longTermFuelTrimBank_2();
    case 9:  return myELM327.fuelPressure();
    case 10: return myELM327.manifoldPressure();
    case 11: return myELM327.rpm();
    case 12: return myELM327.kph();
    case 13: return myELM327.timingAdvance();
    case 14: return myELM327.intakeAirTemp();
    case 15: return myELM327.mafRate();
    case 16: return myELM327.throttle();
    case 17: return myELM327.commandedSecAirStatus();
    case 18: return myELM327.oxygenSensorsPresent_2banks();

    case 19: return myELM327.obdStandards();
    case 20: return myELM327.oxygenSensorsPresent_4banks();
    case 21: return myELM327.auxInputStatus();
    case 22: return myELM327.runTime();

    case 23: return myELM327.distTravelWithMIL();
    case 24: return myELM327.fuelRailPressure();
    case 25: return myELM327.fuelRailGuagePressure();
    case 26: return myELM327.commandedEGR();
    case 27: return myELM327.egrError();
    case 28: return myELM327.commandedEvapPurge();
    case 29: return myELM327.fuelLevel();
    case 30: return myELM327.warmUpsSinceCodesCleared();
    case 31: return myELM327.distSinceCodesCleared();
    case 32: return myELM327.evapSysVapPressure();
    case 33: return myELM327.absBaroPressure();
    case 34: return myELM327.catTempB1S1();
    case 35: return myELM327.catTempB2S1();
    case 36: return myELM327.catTempB1S2();
    case 37: return myELM327.catTempB2S2();

    case 38: return myELM327.monitorDriveCycleStatus();
    case 39: return myELM327.ctrlModVoltage();
    case 40: return myELM327.absLoad();
    case 41: return myELM327.commandedAirFuelRatio();
    case 42: return myELM327.relativeThrottle();
    case 43: return myELM327.ambientAirTemp();
    case 44: return myELM327.absThrottlePosB();
    case 45: return myELM327.absThrottlePosC();
    case 46: return myELM327.absThrottlePosD();
    case 47: return myELM327.absThrottlePosE();
    case 48: return myELM327.absThrottlePosF();
    case 49: return myELM327.commandedThrottleActuator();
    case 50: return myELM327.timeRunWithMIL();
    case 51: return myELM327.timeSinceCodesCleared();
    case 52: return myELM327.maxMafRate();
    case 53: return myELM327.fuelType();
    case 54: return myELM327.ethanolPercent();
    case 55: return myELM327.absEvapSysVapPressure();
    case 56: return myELM327.evapSysVapPressure2();
    case 57: return myELM327.absFuelRailPressure();
    case 58: return myELM327.relativePedalPos();
    case 59: return myELM327.hybridBatLife();
    case 60: return myELM327.oilTemp();
    case 61: return myELM327.fuelInjectTiming();
    case 62: return myELM327.fuelRate();
    case 63: return myELM327.emissionRqmts();

    case 64: return myELM327.demandedTorque();
    case 65: return myELM327.torque();
    case 66: return myELM327.referenceTorque();
    case 67: return myELM327.auxSupported();
  }

  return 0;
}

void goToNextSupportedLivePID() {
  int tries = 0;

  do {
    currentLivePID++;
    if (currentLivePID >= LIVE_PID_COUNT) {
      currentLivePID = 0;
    }

    tries++;
  } while (!livePIDs[currentLivePID].supported && tries < LIVE_PID_COUNT);
}

void updateLiveDataELMduino() {
  if (!checkELMConnection()) {
    drawMessage("Date live", "ELM327 deconectat");
    return;
  }

  bool anySupported = false;

  for (int i = 0; i < LIVE_PID_COUNT; i++) {
    if (livePIDs[i].supported) {
      anySupported = true;
      break;
    }
  }

  if (!anySupported) {
    drawMessage("Date live", "Nu exista PID-uri suportate.");
    return;
  }

  if (!livePIDs[currentLivePID].supported) {
    goToNextSupportedLivePID();
    return;
  }

  float tempValue = queryLivePIDByIndex(currentLivePID);

  if (myELM327.nb_rx_state == ELM_SUCCESS) {
    livePIDs[currentLivePID].value = tempValue;
    livePIDs[currentLivePID].hasValue = true;
    goToNextSupportedLivePID();
  }
  else if (myELM327.nb_rx_state != ELM_GETTING_MSG) {
    livePIDs[currentLivePID].hasValue = false;
    goToNextSupportedLivePID();
  }
}

int liveScrollOffset = 0;
const int LIVE_ROWS_ON_SCREEN = 8;

int getSupportedPIDCount() {
  int count = 0;

  for (int i = 0; i < LIVE_PID_COUNT; i++) {
    if (livePIDs[i].supported) {
      count++;
    }
  }

  return count;
}



void drawLiveDataScreen() {
  drawHeader("Date live OBD-II");

  int supportedCount = getSupportedPIDCount();

  if (supportedCount == 0) {
    tft.setTextSize(2);
    tft.setTextColor(ILI9341_RED);
    tft.setCursor(10, 70);
    tft.print("Niciun PID suportat");
    drawFooter("*=Inapoi");
    return;
  }

  int maxOffset = supportedCount - LIVE_ROWS_ON_SCREEN;
  if (maxOffset < 0) maxOffset = 0;

  if (liveScrollOffset < 0) liveScrollOffset = 0;
  if (liveScrollOffset > maxOffset) liveScrollOffset = maxOffset;

  int y = 42;
  int shown = 0;
  int skipped = 0;

  tft.setTextSize(2);

  for (int i = 0; i < LIVE_PID_COUNT; i++) {
    if (!livePIDs[i].supported) continue;

    if (skipped < liveScrollOffset) {
      skipped++;
      continue;
    }

    tft.setCursor(8, y);

    if (livePIDs[i].hasValue) {
      tft.setTextColor(ILI9341_WHITE);
      tft.print(livePIDs[i].label);
      tft.print(": ");
      tft.print(livePIDs[i].value, livePIDs[i].decimals);
      tft.print(" ");
      tft.print(livePIDs[i].unit);
    } else {
      tft.setTextColor(ILI9341_DARKGREY);
      tft.print(livePIDs[i].label);
      tft.print(": ...");
    }

    y += 21;
    shown++;

    if (shown >= LIVE_ROWS_ON_SCREEN) break;
  }

  tft.setTextSize(1);
  tft.setTextColor(ILI9341_YELLOW);
  tft.setCursor(240, 225);
  tft.print(liveScrollOffset + 1);
  tft.print("-");
  tft.print(liveScrollOffset + shown);
  tft.print("/");
  tft.print(supportedCount);

  drawFooter("2=Sus  8=Jos  *=Inapoi");
}

// ================= DTC =================
void drawDTCs() {
  drawHeader("Coduri DTC");

  if (!checkELMConnection()) {
    drawMessage("DTC", "ELM327 deconectat");
    return;
  }

  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(10, 55);
  tft.print("Se citesc DTC...");

  myELM327.currentDTCCodes(true);

  drawHeader("Coduri DTC");

  uint8_t count = myELM327.DTC_Response.codesFound;

  if (count == 0) {
    tft.setTextSize(2);
    tft.setTextColor(ILI9341_WHITE);
    tft.setCursor(10, 65);
    tft.print("Fara coduri DTC");
    drawFooter("*=Inapoi");
    return;
  }

  int y = 48;
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE);

  for (int i = 0; i < count && i < 7; i++) {
    tft.setCursor(10, y);
    tft.print(myELM327.DTC_Response.codes[i]);
    y += 25;
  }

  drawFooter("*=Inapoi");
}

void clearDTCs() {
  drawHeader("Stergere DTC");
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(10, 65);
  tft.print("Se sterg DTC...");

  bool ok = myELM327.resetDTC();

  drawHeader("Stergere DTC");
  tft.setTextSize(2);

  if (ok) {
    tft.setTextColor(ILI9341_WHITE);
    tft.setCursor(10, 65);
    tft.print("Comanda trimisa");
  } else {
    tft.setTextColor(ILI9341_RED);
    tft.setCursor(10, 65);
    tft.print("Eroare stergere");
  }

  drawFooter("*=Inapoi");
}

// ================= VIN / PROTOCOL =================
String readRawELMCommand(const String& cmd, uint32_t timeout = 4000) {
  String response = "";

  if (!checkELMConnection()) {
    return "";
  }

  elmClient.print(cmd);
  elmClient.print("\r");

  unsigned long start = millis();

  while (millis() - start < timeout) {
    while (elmClient.available()) {
      char c = elmClient.read();
      response += c;

      if (c == '>') {
        return response;
      }
    }

    delay(1);
  }

  return response;
}

String cleanELMResponse(String r) {
  r.replace("\r", "");
  r.replace("\n", "");
  r.replace(" ", "");
  r.replace(">", "");
  r.replace("SEARCHING...", "");
  r.toUpperCase();
  return r;
}

bool isHexCharVIN(char c) {
  return (c >= '0' && c <= '9') ||
         (c >= 'A' && c <= 'F');
}

bool isHexPairVIN(const String& s) {
  if (s.length() != 2) return false;
  return isHexCharVIN(s[0]) && isHexCharVIN(s[1]);
}

uint8_t hexPairToByteVIN(const String& s) {
  return (uint8_t) strtoul(s.c_str(), NULL, 16);
}

String readVIN() {
  if (!checkELMConnection()) {
    return "ELM327 deconectat";
  }

  String raw = readRawELMCommand("0902", 5000);
  String data = cleanELMResponse(raw);

  if (data.length() == 0) {
    return "Fara raspuns VIN";
  }

  String vin = "";
  int searchPos = 0;

  while (vin.length() < 17) {
    int idx = data.indexOf("4902", searchPos);

    if (idx < 0) {
      break;
    }

    // Format raspuns: 49 02 XX ...
    // XX este indexul cadrului, deci sarim peste 49 02 XX
    int pos = idx + 6;

    while (pos + 2 <= data.length() && vin.length() < 17) {
      if (data.substring(pos, pos + 4) == "4902") {
        break;
      }

      String pair = data.substring(pos, pos + 2);

      if (!isHexPairVIN(pair)) {
        break;
      }

      uint8_t b = hexPairToByteVIN(pair);

      if (b >= 32 && b <= 126) {
        vin += (char)b;
      }

      pos += 2;
    }

    searchPos = idx + 4;
  }

  if (vin.length() == 17) {
    return vin;
  }

  return "VIN indisponibil";
}

String readProtocol() {
  if (!checkELMConnection()) return "ELM327 deconectat";

  int8_t status = myELM327.sendCommand_Blocking(DISP_CURRENT_PROTOCOL);

  if (status == ELM_SUCCESS && myELM327.payload != nullptr) {
    String protocol = String(myELM327.payload);
    protocol.trim();

    if (protocol.length() > 0) {
      return protocol;
    }
  }

  return "Protocol indisponibil";
}

void drawSupportedPIDs() {
  drawHeader("PID-uri suportate");

  int y = 45;
  tft.setTextSize(2);

  for (int i = 0; i < LIVE_PID_COUNT; i++) {
    tft.setCursor(10, y);

    if (livePIDs[i].supported) {
      tft.setTextColor(ILI9341_GREEN);
      tft.print("OK  ");
    } else {
      tft.setTextColor(ILI9341_RED);
      tft.print("NO  ");
    }

    tft.setTextColor(ILI9341_WHITE);
    tft.print(livePIDs[i].label);

    y += 22;
    if (y > 205) break;
  }

  drawFooter("*=Inapoi");
}

// ================= DEEP SLEEP =================
//Eliminat
// ================= SCREEN MANAGEMENT =================
int getCurrentMenuCount() {
  switch (currentScreen) {
    case SCREEN_MAIN: return 4;
    case SCREEN_MULTIBRAND: return elmConnected ? 4 : 2;
    case SCREEN_ELM_DIAG: return 7;
    case SCREEN_RENAULT: return 3;
    case SCREEN_SETTINGS: return 2;
    default: return 0;
  }
}

void drawScreen() {
  switch (currentScreen) {
    case SCREEN_MAIN:
      drawMenu("VehicleBF", mainMenu, 4);
      break;

    case SCREEN_MULTIBRAND:
      if (elmConnected) {
        drawMenu("Mod Multimarca", multiMenuConnected, 4);
      } else {
        drawMenu("Mod Multimarca", multiMenuDisconnected, 2);
      }
      break;

    case SCREEN_ELM_DIAG:
      drawMenu("Diagnoza ELM327", elmDiagMenu, 7);
      break;

    case SCREEN_RENAULT:
      drawMenu("Renault Logan 2", renaultMenu, 3);
      break;

    case SCREEN_SETTINGS:
      drawMenu("Setari", settingsMenu, 2);
      break;

    case SCREEN_BRIGHTNESS:
      drawBrightnessScreen();
      break;

    case SCREEN_INFO:
      drawHeader("Info sistem");

      tft.setTextSize(2);
      tft.setTextColor(ILI9341_WHITE);

      tft.setCursor(10, 50);
      tft.print("VehicleBF");

      tft.setCursor(10, 80);
      tft.print("ESP32-S3");

      tft.setCursor(10, 110);
      tft.print("TFT ILI9341");

      tft.setCursor(10, 140);
      tft.print("ELMduino Wi-Fi");

      tft.setCursor(10, 170);
      tft.print("By Serban Albert");

      drawFooter("*=Inapoi");
      break;

    case SCREEN_LIVE_DATA:
      drawLiveDataScreen();
      break;

    case SCREEN_DTC:
      drawDTCs();
      break;

    case SCREEN_CLEAR_DTC_CONFIRM:
      drawHeader("Stergere DTC");

      tft.setTextSize(2);
      tft.setTextColor(ILI9341_RED);
      tft.setCursor(10, 55);
      tft.println("ATENTIE!");

      tft.setTextColor(ILI9341_WHITE);
      tft.setCursor(10, 90);
      tft.println("Se vor sterge");
      tft.setCursor(10, 120);
      tft.println("codurile DTC.");

      drawFooter("5=Confirma  *=Anuleaza");
      break;

    case SCREEN_VIN:
      drawMessage("VIN vehicul", readVIN());
      break;

    case SCREEN_PROTOCOL:
      drawMessage("Protocol OBD", readProtocol());
      break;

    case SCREEN_SUPPORTED_PIDS:
      drawSupportedPIDs();
      break;

    case SCREEN_RENAULT_CAN:
      drawRenaultCANScreen();
      break;
  }
}

void setScreen(ScreenState newScreen) {
  currentScreen = newScreen;
  selectedIndex = 0;
  drawScreen();
}

void goBack() {
  switch (currentScreen) {
    case SCREEN_MULTIBRAND:
    case SCREEN_RENAULT:
    case SCREEN_SETTINGS:
    case SCREEN_INFO:
      setScreen(SCREEN_MAIN);
      break;

    case SCREEN_BRIGHTNESS:
      setScreen(SCREEN_SETTINGS);
      break;

    case SCREEN_ELM_DIAG:
      setScreen(SCREEN_MULTIBRAND);
      break;

    case SCREEN_LIVE_DATA:
    case SCREEN_DTC:
    case SCREEN_CLEAR_DTC_CONFIRM:
    case SCREEN_VIN:
    case SCREEN_PROTOCOL:
    case SCREEN_SUPPORTED_PIDS:
      setScreen(SCREEN_ELM_DIAG);
      break;

    default:
      setScreen(SCREEN_MAIN);
      break;
  }
}

void handleEnter() {
  switch (currentScreen) {
    case SCREEN_MAIN:
      if (selectedIndex == 0) setScreen(SCREEN_MULTIBRAND);
      else if (selectedIndex == 1) setScreen(SCREEN_RENAULT);
      else if (selectedIndex == 2) setScreen(SCREEN_SETTINGS);
      else if (selectedIndex == 3) setScreen(SCREEN_INFO);
      break;

    case SCREEN_MULTIBRAND:
      if (!elmConnected) {
        if (selectedIndex == 0) {
          if (connectToELM327()) {
            setScreen(SCREEN_ELM_DIAG);
          } else {
            setScreen(SCREEN_MULTIBRAND);
          }
        } else if (selectedIndex == 1) {
          setScreen(SCREEN_MAIN);
        }
      } else {
        if (selectedIndex == 0) {
          setScreen(SCREEN_ELM_DIAG);
        } else if (selectedIndex == 1) {
          if (connectToELM327()) {
            setScreen(SCREEN_ELM_DIAG);
          } else {
            setScreen(SCREEN_MULTIBRAND);
          }
        } else if (selectedIndex == 2) {
          disconnectELM327();
          setScreen(SCREEN_MULTIBRAND);
        } else if (selectedIndex == 3) {
          setScreen(SCREEN_MAIN);
        }
      }
      break;

    case SCREEN_ELM_DIAG:
      if (!checkELMConnection()) {
        drawMessage("ELM327", "Conexiune pierduta");
        return;
      }

      if (selectedIndex == 0) {
                                liveScrollOffset = 0;
                                setScreen(SCREEN_LIVE_DATA);
                              }
      else if (selectedIndex == 1) setScreen(SCREEN_DTC);
      else if (selectedIndex == 2) setScreen(SCREEN_CLEAR_DTC_CONFIRM);
      else if (selectedIndex == 3) setScreen(SCREEN_VIN);
      else if (selectedIndex == 4) setScreen(SCREEN_PROTOCOL);
      else if (selectedIndex == 5) setScreen(SCREEN_SUPPORTED_PIDS);
      else if (selectedIndex == 6) setScreen(SCREEN_MULTIBRAND);
      break;

    case SCREEN_CLEAR_DTC_CONFIRM:
      clearDTCs();
      break;

    case SCREEN_RENAULT:
      if (selectedIndex == 2) {
        setScreen(SCREEN_MAIN);
      } else {
        drawMessage("Renault Logan 2", "Functie in lucru");
      }
      break;

    case SCREEN_SETTINGS:
      if (selectedIndex == 0) {
        setScreen(SCREEN_BRIGHTNESS);
      } else if (selectedIndex == 1) {
        setScreen(SCREEN_MAIN);
      }
      break;

    default:
      break;
  }
}

void handleKeypad() {
  char key = keypad.getKey();
  if (!key) return;

  if (key == KEY_SLEEP) {
    //enterDeepSleep(); Functia de deep sleep a fost eliminata
    return;
  }

  if (key == KEY_BACK) {
    goBack();
    return;
  }

  if (currentScreen == SCREEN_BRIGHTNESS) {
    if (key == KEY_UP) {
      brightnessLevel++;
      if (brightnessLevel >= brightnessCount) {
        brightnessLevel = brightnessCount - 1;
      }

      applyBrightness();
      drawBrightnessScreen();
    }

    if (key == KEY_DOWN) {
      brightnessLevel--;
      if (brightnessLevel < 0) {
        brightnessLevel = 0;
      }

      applyBrightness();
      drawBrightnessScreen();
    }

    return;
  }

  if (currentScreen == SCREEN_LIVE_DATA) {
    int supportedCount = getSupportedPIDCount();
    int maxOffset = supportedCount - LIVE_ROWS_ON_SCREEN;
    if (maxOffset < 0) maxOffset = 0;

    if (key == KEY_UP) {
    liveScrollOffset--;
    if (liveScrollOffset < 0) liveScrollOffset = 0;
    drawLiveDataScreen();
    }

    if (key == KEY_DOWN) {
    liveScrollOffset++;
    if (liveScrollOffset > maxOffset) liveScrollOffset = maxOffset;
    drawLiveDataScreen();
    }

  return;
  }

  int count = getCurrentMenuCount();

  if (key == KEY_UP && count > 0) {
    selectedIndex--;
    if (selectedIndex < 0) selectedIndex = count - 1;
    drawScreen();
  }

  if (key == KEY_DOWN && count > 0) {
    selectedIndex++;
    if (selectedIndex >= count) selectedIndex = 0;
    drawScreen();
  }

  if (key == KEY_ENTER) {
    handleEnter();
  }


  if (currentScreen == SCREEN_RENAULT && key == KEY_ENTER) {
  if (selectedIndex == 0 || selectedIndex == 1) {
    if (!canStarted) {
      drawStatus("CAN", "Initializare CAN...");
      if (!initCAN500k()) {
        drawMessage("CAN", "Eroare initializare CAN");
        return;
      }
    }

    currentScreen = SCREEN_RENAULT_CAN;
    selectedIndex = 0;
    drawScreen();
  }

  if (selectedIndex == 2) {
    currentScreen = SCREEN_MAIN;
    selectedIndex = 0;
    drawScreen();
  }
  }
}

// ================= SETUP=======================================================
void setup() {
  Serial.begin(115200);

  pinMode(TFT_BL, OUTPUT);
  applyBrightness();

  pinMode(RGB_LED, OUTPUT);
  digitalWrite(RGB_LED, LOW);
  neopixelWrite(RGB_LED, 0, 0, 0);

  SPI.begin(TFT_SCK, TFT_MISO, TFT_MOSI, TFT_CS);

  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(ILI9341_BLACK);

  drawHeader("VehicleBF");

  tft.setTextSize(3);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(55, 85);
  tft.print("VehicleBF");

  tft.setTextSize(1);
  tft.setCursor(110, 128);
  tft.print("By Serban Albert");

  delay(1500);

  setScreen(SCREEN_MAIN);
}

// ================= LOOP=======================================================

void loop() {
  handleKeypad();

  if (currentScreen == SCREEN_LIVE_DATA) {
    updateLiveDataELMduino();

    if (millis() - lastLiveDraw > 700) {
      lastLiveDraw = millis();
      drawLiveDataScreen();
    }
  }

  if (currentScreen == SCREEN_RENAULT_CAN) {
    readCANMessages();

    static unsigned long lastRenaultDraw = 0;

    if (millis() - lastRenaultDraw > 250) {
      drawRenaultCANScreen();
      lastRenaultDraw = millis();
    }
  }
}