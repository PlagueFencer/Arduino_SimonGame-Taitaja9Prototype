// Web panel for the Simon game.
//
// The ESP32 keeps the score and leaderboard and shares them two ways:
//   1. Wi-Fi hotspot: connect to "SimonGame" and open http://192.168.4.1
//   2. USB serial:    open panel.html in Chrome/Edge on the PC and click "Connect USB"
//
// Over USB the panel reads lines that start with "@" (JSON state) and ignores
// the normal debug messages. It can send back "@state" and "@clear".
//
// Uses only libraries that come with the ESP32 core (no extra installs).

#pragma once
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include "panel_html.h"  // generated from panel.html by update_panel.ps1

// ---------- Settings ----------

const char* AP_SSID = "SimonGame";
const char* AP_PASS = "simon1234";   // at least 8 characters

const int POINTS_PER_COLOR = 50;     // every correct button press
const int POINTS_PER_ROUND = 100;    // bonus for a completed round...
const int FREE_ROUNDS      = 2;      // ...but not for the first 2 rounds
const int BOARD_SIZE       = 10;     // leaderboard length

// ---------- State ----------

struct ScoreEntry {
  uint32_t score;
  uint32_t rounds;
  uint32_t game;   // game number, so entries can be told apart
};

WebServer server(80);
Preferences prefs;

ScoreEntry board[BOARD_SIZE];
int boardCount = 0;
uint32_t gamesPlayed = 0;

const char* panelState = "idle";     // idle | starting | showing | input | over
int  panelRound = 0;
long panelScore = 0;
long lastScore  = -1;                // -1 = no game finished yet
int  lastRounds = 0;

String serialLine;

// ---------- JSON + sending ----------

String panelStateJson() {
  String j = "{\"state\":\"";
  j += panelState;
  j += "\",\"round\":";   j += panelRound;
  j += ",\"score\":";     j += panelScore;
  j += ",\"best\":";      j += (boardCount > 0 ? board[0].score : 0);
  j += ",\"games\":";     j += gamesPlayed;
  j += ",\"lastScore\":"; j += lastScore;
  j += ",\"lastRounds\":";j += lastRounds;
  j += ",\"board\":[";
  for (int i = 0; i < boardCount; i++) {
    if (i) j += ",";
    j += "{\"s\":"; j += board[i].score;
    j += ",\"r\":"; j += board[i].rounds;
    j += ",\"g\":"; j += board[i].game;
    j += "}";
  }
  j += "]}";
  return j;
}

// Sends the state over USB. Hotspot viewers fetch /state on their own.
void panelChanged() {
  Serial.print("@");
  Serial.println(panelStateJson());
}

// ---------- Leaderboard storage (survives power-off) ----------

void loadBoard() {
  prefs.begin("simon", false);
  gamesPlayed = prefs.getUInt("games", 0);
  boardCount = 0;
  if (prefs.isKey("board")) {
    boardCount = prefs.getBytes("board", board, sizeof(board)) / sizeof(ScoreEntry);
  }
}

void saveBoard() {
  prefs.putUInt("games", gamesPlayed);
  prefs.putBytes("board", board, boardCount * sizeof(ScoreEntry));
}

void clearBoard() {
  boardCount = 0;
  gamesPlayed = 0;
  lastScore = -1;
  lastRounds = 0;
  prefs.remove("board");
  prefs.putUInt("games", 0);
  Serial.println("[PANEL] Leaderboard cleared");
  panelChanged();
}

// Puts a finished game into the sorted top list (if it is good enough).
void addToBoard(long score, int rounds) {
  gamesPlayed++;
  if (score > 0) {
    int pos = boardCount;
    while (pos > 0 && board[pos - 1].score < (uint32_t)score) pos--;
    if (pos < BOARD_SIZE) {
      int last = (boardCount < BOARD_SIZE) ? boardCount : BOARD_SIZE - 1;
      for (int i = last; i > pos; i--) board[i] = board[i - 1];
      board[pos] = { (uint32_t)score, (uint32_t)rounds, gamesPlayed };
      if (boardCount < BOARD_SIZE) boardCount++;
    }
  }
  saveBoard();
}

// ---------- Calls from the game ----------

void panelGameStart() {
  panelScore = 0;
  panelRound = 0;
  panelState = "starting";
  panelChanged();
}

void panelRoundStart(int round) {
  panelRound = round;
  panelState = "showing";
  panelChanged();
}

void panelInputStart() {
  panelState = "input";
  panelChanged();
}

void panelCorrectPress() {
  panelScore += POINTS_PER_COLOR;
  panelChanged();
}

void panelRoundDone(int round) {
  if (round > FREE_ROUNDS) {
    panelScore += POINTS_PER_ROUND;
    panelChanged();
  }
}

// Game ended by a mistake, a timeout or by beating all rounds.
void panelGameOver(int roundsCompleted) {
  lastScore = panelScore;
  lastRounds = roundsCompleted;
  addToBoard(panelScore, roundsCompleted);
  panelState = "over";
  Serial.print("[PANEL] Final score: ");
  Serial.println(panelScore);
  panelChanged();
}

// Reset button pressed mid-game: the game is not recorded.
void panelGameAborted() {
  panelState = "idle";
  panelRound = 0;
  panelScore = 0;
  panelChanged();
}

// ---------- Setup + polling ----------

void handleSerialCommand(const String& cmd) {
  if (cmd == "@clear") clearBoard();
  else if (cmd == "@state") panelChanged();
}

// Call this often (inside every waiting loop) so the panel stays responsive.
void panelPoll() {
  server.handleClient();
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (serialLine.length()) handleSerialCommand(serialLine);
      serialLine = "";
    } else if (serialLine.length() < 32) {
      serialLine += c;
    }
  }
}

void panelBegin() {
  loadBoard();

  WiFi.softAP(AP_SSID, AP_PASS);
  Serial.print("[PANEL] Hotspot \"");
  Serial.print(AP_SSID);
  Serial.print("\" up. Open http://");
  Serial.println(WiFi.softAPIP());

  server.on("/", []() {
    server.send(200, "text/html", PANEL_HTML);
  });
  server.on("/state", []() {
    server.sendHeader("Cache-Control", "no-store");
    server.send(200, "application/json", panelStateJson());
  });
  server.on("/clear", HTTP_POST, []() {
    clearBoard();
    server.send(200, "application/json", panelStateJson());
  });
  server.begin();

  panelChanged();
}
