// Simon game for ESP32
// 4 buttons + 4 LEDs + 1 reset button.
// Watch the LED sequence, then repeat it with the buttons.
// Each round adds one more step. A wrong press or timeout ends the game.
// Press the reset button at any time to start a new game.
// An active buzzer gives a long buzz when the player fails.
// Scores and the leaderboard are shown on a web panel (see web_panel.h).

#include "web_panel.h"

const int buttonPIN_1 = A4;
const int buttonPIN_2 = A3;
const int buttonPIN_3 = A2;
const int buttonPIN_4 = A1;
const int ResetButtonPIN = 15;
const int buzzerPin = A5;
const int resolution = 8;     // 8-bit PWM resolution for the buzzer
const int BUZZER_FREQ = 2000; // buzzer tone in Hz

const int ledpin1 = 27;
const int ledpin2 = 32;
const int ledpin3 = 14;
const int ledpin4 = 33;

// Button i lights LED i
const int buttonPins[4] = {buttonPIN_1, buttonPIN_2, buttonPIN_3, buttonPIN_4};
const int ledPins[4]    = {ledpin1, ledpin2, ledpin3, ledpin4};

// Buttons read HIGH when pressed (as in the original test code).
const int BUTTON_PRESSED = HIGH;

// If your LEDs light up the wrong way round (on when they should be off),
// swap these two values.
const int LED_ON  = LOW;
const int LED_OFF = HIGH;

// Active buzzer (pin is buzzerPin at the top).
// If your buzzer module beeps on LOW instead, swap these two values.
const int BUZZER_ON  = HIGH;
const int BUZZER_OFF = LOW;
const int FAIL_BUZZ_MS = 1000;           // length of the "you failed" buzz

// Set to false to turn off the extra Serial debug messages.
const bool DEBUG = true;

const int MAX_STEPS = 100;               // longest possible sequence
const unsigned long INPUT_TIMEOUT = 5000; // ms the player has for each press
const int DEBOUNCE_MS = 30;

int sequence[MAX_STEPS];
int sequenceLength = 0;

// ---------- LED helpers ----------

void ledOn(int i)  { digitalWrite(ledPins[i], LED_ON); }
void ledOff(int i) { digitalWrite(ledPins[i], LED_OFF); }

void allLeds(bool on) {
  for (int i = 0; i < 4; i++) {
    digitalWrite(ledPins[i], on ? LED_ON : LED_OFF);
  }
}

// ---------- Buzzer helpers ----------

// The buzzer pin is driven with PWM (ledc), set up once in setup().
// Don't use digitalWrite/pinMode on buzzerPin: in ESP32 core 3.x that
// disconnects the pin from PWM and the buzzer goes silent.

void buzzerOn() {
  ledcWriteTone(buzzerPin, BUZZER_FREQ);
  if (DEBUG) {
    Serial.print("[BUZZER] ON  (pin ");
    Serial.print(buzzerPin);
    Serial.print(", ");
    Serial.print(BUZZER_FREQ);
    Serial.println(" Hz)");
  }
}

void buzzerOff() {
  ledcWriteTone(buzzerPin, 0);
  if (DEBUG) {
    Serial.print("[BUZZER] OFF (pin ");
    Serial.print(buzzerPin);
    Serial.println(")");
  }
}

// Short test beep at startup so you can hear the buzzer is wired right.
void buzzerSelfTest() {
  Serial.println("[BUZZER] Self-test: you should hear a short beep now");
  buzzerOn();
  delay(150);
  buzzerOff();
  Serial.println("[BUZZER] Self-test done. No beep? Check wiring, polarity,");
  Serial.println("[BUZZER]   the pin number, or swap BUZZER_ON / BUZZER_OFF.");
}

// ---------- Button helpers ----------

bool isPressed(int pin) {
  return digitalRead(pin) == BUTTON_PRESSED;
}

bool resetPressed() {
  return isPressed(ResetButtonPIN);
}

// Waits like delay(), but returns true early if reset is pressed.
bool waitOrReset(unsigned long ms) {
  unsigned long start = millis();
  while (millis() - start < ms) {
    if (resetPressed()) return true;
    panelPoll();
    delay(1);
  }
  return false;
}

// Return codes for readPlayerButton()
const int RESULT_TIMEOUT = -1;
const int RESULT_RESET   = -2;

// Waits for the player to press and release one of the 4 buttons.
// Lights the matching LED while held. Returns 0-3, or a RESULT_ code.
int readPlayerButton() {
  unsigned long start = millis();

  while (millis() - start < INPUT_TIMEOUT) {
    if (resetPressed()) return RESULT_RESET;
    panelPoll();

    for (int i = 0; i < 4; i++) {
      if (isPressed(buttonPins[i])) {
        delay(DEBOUNCE_MS);
        if (!isPressed(buttonPins[i])) continue; // was just noise

        if (DEBUG) {
          Serial.print("[INPUT] Button ");
          Serial.print(i + 1);
          Serial.println(" pressed");
        }
        ledOn(i);
        while (isPressed(buttonPins[i])) {
          delay(1);
        }
        ledOff(i);
        delay(DEBOUNCE_MS);
        return i;
      }
    }
  }
  if (DEBUG) Serial.println("[INPUT] No button pressed in time");
  return RESULT_TIMEOUT;
}

// ---------- Game parts ----------

// Speed up as the sequence gets longer (min 150 ms per step).
int stepTime() {
  int t = 500 - sequenceLength * 20;
  return t < 150 ? 150 : t;
}

// Shows the current sequence. Returns true if reset was pressed.
bool playSequence() {
  int t = stepTime();
  for (int i = 0; i < sequenceLength; i++) {
    ledOn(sequence[i]);
    if (waitOrReset(t)) { allLeds(false); return true; }
    ledOff(sequence[i]);
    if (waitOrReset(t / 2)) return true;
  }
  return false;
}

void startAnimation() {
  for (int round = 0; round < 2; round++) {
    for (int i = 0; i < 4; i++) {
      ledOn(i);
      delay(80);
      ledOff(i);
    }
  }
  delay(500);
}

void winRoundAnimation() {
  delay(200);
  allLeds(true);
  delay(150);
  allLeds(false);
  delay(400);
}

void gameOverAnimation() {
  Serial.print("Game over! Score: ");
  Serial.println(sequenceLength - 1);

  // Long buzz with all LEDs on
  if (DEBUG) {
    Serial.print("[BUZZER] Fail buzz for ");
    Serial.print(FAIL_BUZZ_MS);
    Serial.println(" ms");
  }
  allLeds(true);
  buzzerOn();
  delay(FAIL_BUZZ_MS);
  buzzerOff();
  allLeds(false);
  delay(300);

  for (int i = 0; i < 4; i++) {
    allLeds(true);
    delay(200);
    allLeds(false);
    delay(200);
  }
}

// Idle: blinks the LEDs one by one until any button is pressed.
void waitForStart() {
  Serial.println("Press any button to start");
  int i = 0;
  while (true) {
    ledOn(i);
    for (int t = 0; t < 200; t++) {
      bool anyPressed = resetPressed();
      for (int b = 0; b < 4; b++) {
        if (isPressed(buttonPins[b])) anyPressed = true;
      }
      if (anyPressed) {
        allLeds(false);
        // wait until everything is released
        bool held = true;
        while (held) {
          held = resetPressed();
          for (int b = 0; b < 4; b++) {
            if (isPressed(buttonPins[b])) held = true;
          }
          delay(5);
        }
        delay(300);
        return;
      }
      panelPoll();
      delay(1);
    }
    ledOff(i);
    i = (i + 1) % 4;
  }
}

void waitForResetRelease() {
  allLeds(false);
  ledcWriteTone(buzzerPin, 0); // make sure the buzzer never stays on
  while (resetPressed()) delay(5);
  delay(DEBOUNCE_MS);
}

// Plays one full game. Returns when the game ends or reset is pressed.
void playGame() {
  sequenceLength = 0;
  panelGameStart();
  startAnimation();

  while (sequenceLength < MAX_STEPS) {
    // Add a new random step
    sequence[sequenceLength] = random(4);
    sequenceLength++;
    Serial.print("Round ");
    Serial.println(sequenceLength);
    if (DEBUG) {
      Serial.print("[GAME] Sequence:");
      for (int s = 0; s < sequenceLength; s++) {
        Serial.print(" ");
        Serial.print(sequence[s] + 1);
      }
      Serial.println();
    }
    panelRoundStart(sequenceLength);

    if (playSequence()) { panelGameAborted(); waitForResetRelease(); return; }
    panelInputStart();

    // Player repeats the sequence
    for (int i = 0; i < sequenceLength; i++) {
      int pressed = readPlayerButton();

      if (pressed == RESULT_RESET) {
        Serial.println("Reset");
        panelGameAborted();
        waitForResetRelease();
        return;
      }
      if (pressed == RESULT_TIMEOUT) {
        Serial.println("Failed: too slow");
        panelGameOver(sequenceLength - 1);
        gameOverAnimation();
        return;
      }
      if (pressed != sequence[i]) {
        Serial.print("Failed: step ");
        Serial.print(i + 1);
        Serial.print(" expected button ");
        Serial.print(sequence[i] + 1);
        Serial.print(", got button ");
        Serial.println(pressed + 1);
        panelGameOver(sequenceLength - 1);
        gameOverAnimation();
        return;
      }
      panelCorrectPress();
    }

    panelRoundDone(sequenceLength);
    winRoundAnimation();
  }

  Serial.println("You beat the whole game!");
  panelGameOver(sequenceLength);
  for (int i = 0; i < 3; i++) startAnimation();
}

// ---------- Arduino entry points ----------

void setup() {
  Serial.setTxBufferSize(1024); // room for the panel's score messages
  Serial.begin(115200);
  Serial.println("ESP32 Simon game starting");

  for (int i = 0; i < 4; i++) {
    pinMode(buttonPins[i], INPUT);
    pinMode(ledPins[i], OUTPUT);
  }
  pinMode(ResetButtonPIN, INPUT);

  // Set up PWM on the buzzer pin (ESP32 core 3.x API)
  if (ledcAttach(buzzerPin, BUZZER_FREQ, resolution)) {
    Serial.println("[BUZZER] PWM attached OK");
  } else {
    Serial.println("[BUZZER] ERROR: ledcAttach failed - check buzzerPin");
  }
  ledcWriteTone(buzzerPin, 0); // start silent

  allLeds(false);
  buzzerSelfTest();
  randomSeed(esp_random()); // ESP32 hardware random number
  panelBegin();             // Wi-Fi hotspot + web panel
}

void loop() {
  waitForStart();
  playGame();
  // Play a tone
  ledcWriteTone(buzzerPin, 1000);  // Play 1kHz tone
  delay(500);                      // Beep duration
  ledcWriteTone(buzzerPin, 0);     // Stop sound
}
