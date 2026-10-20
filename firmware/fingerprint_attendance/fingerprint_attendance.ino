/* ESP32-WROOM-32 fingerprint attendance: new reference implementation.
 * No network, enrollment, template export, names or calendar clock.
 * See hardware/wiring.md; sensor TX must be 3.3V compatible.
 */
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Fingerprint.h>
#include <LittleFS.h>
#include <Preferences.h>
#include "attendance.h"

constexpr int FP_RX = 16; // ESP32 RX <- sensor TX
constexpr int FP_TX = 17; // ESP32 TX -> sensor RX
constexpr int OLED_SDA = 21;
constexpr int OLED_SCL = 22;
constexpr uint8_t OLED_ADDRESS = 0x3C;
constexpr uint32_t FP_BAUD = 57600;
constexpr size_t MAX_LOG_BYTES = 128 * 1024;
constexpr char LOG_FILE[] = "/attendance.csv";
HardwareSerial sensorSerial(2);
Adafruit_Fingerprint finger(&sensorSerial);
Adafruit_SSD1306 oled(128, 64, &Wire, -1);
attendance::Session session;
Preferences prefs;
bool oledReady = false;
bool fsReady = false;
bool storageReady = false;
bool sensorReady = false;
bool waitingRemoval = false;
uint32_t bootSession = 0;
uint32_t lastPoll = 0;
uint32_t statusAt = 0;
bool showingResult = false;
String command;

void show(const char* first, const char* second = "") {
  Serial.printf("STATUS: %s %s\n", first, second);
  if (!oledReady) return;
  oled.clearDisplay(); oled.setTextColor(SSD1306_WHITE);
  oled.setTextSize(1); oled.setCursor(0, 0);
  oled.println(F("FINGERPRINT ATTENDANCE")); oled.println();
  oled.setTextSize(2); oled.println(first);
  oled.setTextSize(1); oled.println(); oled.println(second);
  oled.display();
}
void result(const char* first, const char* second = "") {
  show(first, second); statusAt = millis(); showingResult = true;
}
bool prepareStorage() {
  if (!fsReady) return false;
  if (!LittleFS.exists(LOG_FILE)) {
    File file = LittleFS.open(LOG_FILE, FILE_WRITE);
    if (!file) return false;
    const char header[] = "boot_session,uptime_ms,sensor_id,confidence\n";
    const bool ok = file.print(header) == sizeof(header) - 1;
    file.flush(); file.close();
    if (!ok) return false;
  }
  File file = LittleFS.open(LOG_FILE, FILE_READ);
  if (!file) return false;
  const String header = file.readStringUntil('\n');
  const size_t bytes = file.size(); file.close();
  if (header != "boot_session,uptime_ms,sensor_id,confidence" ||
      bytes >= MAX_LOG_BYTES) return false;
  if (!prefs.begin("attendance", false)) return false;
  const uint32_t prior = prefs.getUInt("boot", 0);
  const uint32_t next = prior + 1;
  const bool ok = prior != UINT32_MAX && prefs.putUInt("boot", next) == sizeof(uint32_t);
  prefs.end(); if (!ok) return false;
  bootSession = next;
  return true;
}
bool appendRecord(uint16_t id, uint16_t confidence) {
  if (!storageReady) return false;
  char row[80];
  const int count = snprintf(row, sizeof(row), "%lu,%lu,%u,%u\n",
    (unsigned long)bootSession, (unsigned long)millis(), id, confidence);
  if (count <= 0 || size_t(count) >= sizeof(row)) return false;
  File file = LittleFS.open(LOG_FILE, FILE_APPEND);
  if (!file) return false;
  if (file.size() + size_t(count) > MAX_LOG_BYTES) { file.close(); return false; }
  const bool ok = file.write((const uint8_t*)row, size_t(count)) == size_t(count);
  file.flush(); file.close();
  return ok; // Not a transaction or power-loss durability guarantee
}
void handleCommand() {
  while (Serial.available()) {
    const char c = char(Serial.read());
    if (c == '\r') continue;
    if (c != '\n') {
      if (command.length() < 40) command += c;
      else command = "INVALID";
      continue;
    }
    command.trim();
    if (command == "INIT STORAGE" && !fsReady) {
      // Explicit maintenance command only. Never auto-format at boot.
      if (LittleFS.format()) {
        show("RESTART", "Storage formatted; press reset");
      } else show("FS ERROR", "Format failed");
    } else if (command == "EXPORT" && fsReady) {
      File file = LittleFS.open(LOG_FILE, FILE_READ);
      if (file) {
        Serial.println(F("BEGIN_ATTENDANCE_CSV"));
        while (file.available()) Serial.write(file.read());
        file.close(); Serial.println(F("END_ATTENDANCE_CSV"));
      } else Serial.println(F("No attendance file"));
    } else Serial.println(F("Commands: EXPORT; INIT STORAGE (only if unmounted)"));
    command = "";
  }
}
void setup() {
  Serial.begin(115200);
  command.reserve(40);
  Wire.begin(OLED_SDA, OLED_SCL);
  Wire.setTimeOut(50);
  Wire.beginTransmission(OLED_ADDRESS);
  const bool displayAck = Wire.endTransmission() == 0;
  oledReady = displayAck && oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS);
  if (!oledReady) Serial.println(F("OLED unavailable; serial fallback enabled"));
  fsReady = LittleFS.begin(false); // No automatic erasure
  storageReady = prepareStorage();
  sensorSerial.begin(FP_BAUD, SERIAL_8N1, FP_RX, FP_TX);
  finger.begin(FP_BAUD);
  sensorReady = finger.verifyPassword();
  if (!storageReady) show("FS ERROR", "Check storage; see setup guide");
  else if (!sensorReady) show("FP ERROR", "Check sensor; reset to retry");
  else show("READY", "Place enrolled finger");
  Serial.printf("Boot session: %lu; timestamps are uptime, not date/time\n",
                (unsigned long)bootSession);
}
void loop() {
  handleCommand();
  if (!storageReady || !sensorReady) { delay(10); return; }
  const uint32_t now = millis();
  if (showingResult && attendance::due(now, statusAt, 2000)) {
    showingResult = false;
    show(waitingRemoval ? "REMOVE" : "READY", "Place enrolled finger after removal");
  }
  if (!attendance::due(now, lastPoll, 100)) { delay(1); return; }
  lastPoll = now;
  const uint8_t image = finger.getImage();
  if (image == FINGERPRINT_NOFINGER) {
    waitingRemoval = false;
    return;
  }
  if (image != FINGERPRINT_OK) {
    if (!showingResult) result("SCAN ERROR", "Check sensor / retry");
    return;
  }
  if (waitingRemoval) return;
  waitingRemoval = true;
  if (finger.image2Tz() != FINGERPRINT_OK) {
    result("RETRY", "Remove and place finger again"); return;
  }
  const uint8_t match = finger.fingerFastSearch();
  if (match == FINGERPRINT_NOTFOUND) { result("NO MATCH", "Not recorded"); return; }
  if (match != FINGERPRINT_OK) { result("FP ERROR", "Not recorded"); return; }
  const uint16_t id = finger.fingerID;
  const attendance::Decision decision = session.check(id);
  if (decision == attendance::INVALID_ID) { result("ID ERROR", "Allowed IDs: 1-127"); return; }
  char detail[32]; snprintf(detail, sizeof(detail), "ID %u / session %lu", id,
                           (unsigned long)bootSession);
  if (decision == attendance::DUPLICATE) { result("ALREADY", detail); return; }
  if (!appendRecord(id, finger.confidence)) {
    storageReady = false; result("FS ERROR", "Not confirmed; export and inspect"); return;
  }
  session.commit(id);
  result("RECORDED", detail);
}
