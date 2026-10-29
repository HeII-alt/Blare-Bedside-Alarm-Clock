#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>

// ---------- Pins (from your schematic) ----------
#define ROW1_PIN   D0
#define ROW2_PIN   D1
#define COL1_PIN   D2
#define COL2_PIN   D3

#define TFT_MOSI   D4   // screen "SDA"
#define TFT_SCK    D5   // screen "SCL"
#define TFT_RST    D6   // screen "RES"
#define TFT_DC     D7
#define TFT_CS     D8
#define TFT_BLK    D9
#define BUZZER_PIN D10

// ---------- Screen size: CHANGE to match your display ----------
#define SCREEN_W 240
#define SCREEN_H 320

Adafruit_ST7789 tft(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCK, TFT_RST);

// ---------- Keys ----------
// Key numbers: 0 = (row1,col1), 1 = (row1,col2), 2 = (row2,col1), 3 = (row2,col2)
// K0 = MODE / NEXT   K1 = UP   K2 = DOWN   K3 = BACK / STOP
const uint8_t rowPins[2] = {ROW1_PIN, ROW2_PIN};
const uint8_t colPins[2] = {COL1_PIN, COL2_PIN};
bool keyDown[4]    = {false, false, false, false};
bool keyLast[4]    = {false, false, false, false};
bool keyPressed[4] = {false, false, false, false};  // true for one loop on press
unsigned long keyChanged[4] = {0, 0, 0, 0};
const unsigned long DEBOUNCE_MS = 25;

void scanKeys() {
  for (uint8_t k = 0; k < 4; k++) keyPressed[k] = false;

  for (uint8_t c = 0; c < 2; c++) {
    digitalWrite(colPins[c], HIGH);
    delayMicroseconds(5);
    for (uint8_t r = 0; r < 2; r++) {
      uint8_t k = r * 2 + c;
      bool reading = digitalRead(rowPins[r]);
      if (reading != keyLast[k]) {
        keyLast[k] = reading;
        keyChanged[k] = millis();
      }
      if ((millis() - keyChanged[k]) > DEBOUNCE_MS && reading != keyDown[k]) {
        keyDown[k] = reading;
        if (reading) keyPressed[k] = true;
      }
    }
    digitalWrite(colPins[c], LOW);
  }
}

// ---------- Clock state ----------
enum Mode { SHOW_CLOCK, SET_TIME, SET_ALARM, RINGING };
Mode mode = SHOW_CLOCK;

uint32_t secOfDay = 7UL * 3600;       // current time (starts 07:00:00 after power-up)
unsigned long lastTick = 0;

uint8_t alarmH = 7, alarmM = 30;
bool alarmOn = true;
int32_t nextRing = -1;                 // seconds-of-day when alarm should ring (-1 = none)
const uint32_t SNOOZE_SEC = 5 * 60;

// editing
uint8_t editH, editM;
uint8_t editField = 0;                 // 0 = hours, 1 = minutes

bool needFull = true;
unsigned long lastDraw = 0;
unsigned long lastBeep = 0;
bool flash = false;

void updateNextRing() {
  nextRing = alarmOn ? (int32_t)(alarmH * 3600UL + alarmM * 60UL) : -1;
}

// ---------- Drawing ----------
void print2(uint8_t v) {
  if (v < 10) tft.print('0');
  tft.print(v);
}

void drawBigTime(uint8_t h, uint8_t m, int highlight) {
  tft.setTextSize(7);
  tft.setTextColor(highlight == 0 ? ST77XX_YELLOW : ST77XX_WHITE, ST77XX_BLACK);
  tft.setCursor(40, 70);
  print2(h);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.setCursor(124, 70);
  tft.print(":");
  tft.setTextColor(highlight == 1 ? ST77XX_YELLOW : ST77XX_WHITE, ST77XX_BLACK);
  tft.setCursor(166, 70);
  print2(m);
}

void drawScreen() {
  uint8_t h = secOfDay / 3600;
  uint8_t m = (secOfDay % 3600) / 60;
  uint8_t s = secOfDay % 60;

  if (mode == RINGING) {
    tft.fillScreen(flash ? ST77XX_RED : ST77XX_BLACK);
    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(5);
    tft.setCursor(35, 60);
    tft.print("WAKE UP!");
    tft.setTextSize(2);
    tft.setCursor(20, 150);
    tft.print("K1 = snooze 5 min");
    tft.setCursor(20, 180);
    tft.print("K4 = stop alarm");
    needFull = true;  // redraw normal screen after
    return;
  }

  if (needFull) {
    tft.fillScreen(ST77XX_BLACK);
    needFull = false;
  }

  tft.setTextSize(2);
  tft.setTextColor(ST77XX_CYAN, ST77XX_BLACK);
  tft.setCursor(10, 10);
  if (mode == SHOW_CLOCK)      tft.print("BLARE                  ");
  else if (mode == SET_TIME)   tft.print("SET TIME               ");
  else                         tft.print("SET ALARM              ");

  if (mode == SHOW_CLOCK) {
    drawBigTime(h, m, -1);
    tft.setTextSize(3);
    tft.setTextColor(ST77XX_GREEN, ST77XX_BLACK);
    tft.setCursor(130, 135);
    print2(s);

    tft.setTextSize(3);
    tft.setTextColor(alarmOn ? ST77XX_ORANGE : ST77XX_DARKGREY, ST77XX_BLACK);
    tft.setCursor(20, 180);
    tft.print("Alarm ");
    print2(alarmH); tft.print(":"); print2(alarmM);
    tft.print(alarmOn ? " ON " : " OFF");

    tft.setTextSize(2);
    tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
    tft.setCursor(10, 225);
    tft.print("K1 time K2 alarm K3 on/off");
  } else {
    drawBigTime(editH, editM, editField);
    tft.setTextSize(2);
    tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
    tft.setCursor(10, 160);
    tft.print("K1 next  K2 +  K3 -");
    tft.setCursor(10, 190);
    tft.print("K4 save");
  }
}

// ---------- Setup ----------
void setup() {
  Serial.begin(115200);

  for (uint8_t c = 0; c < 2; c++) {
    pinMode(colPins[c], OUTPUT);
    digitalWrite(colPins[c], LOW);
  }
  for (uint8_t r = 0; r < 2; r++) {
    pinMode(rowPins[r], INPUT_PULLDOWN);   // diodes: column -> row
  }

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(TFT_BLK, OUTPUT);
  digitalWrite(TFT_BLK, HIGH);

  tft.init(SCREEN_W, SCREEN_H);
  tft.setRotation(1);                      // landscape
  tft.fillScreen(ST77XX_BLACK);

  updateNextRing();
  lastTick = millis();
}

// ---------- Loop ----------
void loop() {
  // 1) keep time
  while (millis() - lastTick >= 1000) {
    lastTick += 1000;
    secOfDay = (secOfDay + 1) % 86400UL;
    if (mode == SHOW_CLOCK && nextRing >= 0 && (int32_t)secOfDay == nextRing) {
      mode = RINGING;
      lastBeep = 0;
    }
  }

  // 2) read keys
  scanKeys();

  switch (mode) {
    case SHOW_CLOCK:
      if (keyPressed[0]) {                 // set time
        editH = secOfDay / 3600;
        editM = (secOfDay % 3600) / 60;
        editField = 0;
        mode = SET_TIME;
        needFull = true;
      } else if (keyPressed[1]) {          // set alarm
        editH = alarmH;
        editM = alarmM;
        editField = 0;
        mode = SET_ALARM;
        needFull = true;
      } else if (keyPressed[2]) {          // alarm on/off
        alarmOn = !alarmOn;
        updateNextRing();
      }
      break;

    case SET_TIME:
    case SET_ALARM:
      if (keyPressed[0]) editField = !editField;
      if (keyPressed[1]) {
        if (editField == 0) editH = (editH + 1) % 24;
        else                editM = (editM + 1) % 60;
      }
      if (keyPressed[2]) {
        if (editField == 0) editH = (editH + 23) % 24;
        else                editM = (editM + 59) % 60;
      }
      if (keyPressed[3]) {                 // save
        if (mode == SET_TIME) {
          secOfDay = editH * 3600UL + editM * 60UL;
          lastTick = millis();
        } else {
          alarmH = editH;
          alarmM = editM;
          alarmOn = true;
          updateNextRing();
        }
        mode = SHOW_CLOCK;
        needFull = true;
      }
      break;

    case RINGING:
      if (millis() - lastBeep > 400) {     // beep pattern
        lastBeep = millis();
        tone(BUZZER_PIN, 2500, 200);
        flash = !flash;
        drawScreen();
      }
      if (keyPressed[0]) {                 // snooze
        noTone(BUZZER_PIN);
        nextRing = (secOfDay + SNOOZE_SEC) % 86400UL;
        mode = SHOW_CLOCK;
        needFull = true;
      } else if (keyPressed[3]) {          // stop
        noTone(BUZZER_PIN);
        updateNextRing();
        mode = SHOW_CLOCK;
        needFull = true;
      }
      break;
  }

  // 3) refresh screen
  if (mode != RINGING && (millis() - lastDraw > 100 || needFull)) {
    lastDraw = millis();
    drawScreen();
  }
}
