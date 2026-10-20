# Installation and bring-up

## Build environment

Arduino IDE: install Espressif's ESP32 board package 3.0.7 and choose ESP32 Dev Module (classic WROOM-32 reference). Use a 4 MB board with a partition scheme containing a filesystem (the default reference scheme); record your actual flash/partition setting. Install the pinned libraries listed in the README. This sketch is not prevalidated for ESP32-S3.

CLI equivalent:

```sh
arduino-cli core update-index --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli core install esp32:esp32@3.0.7 --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli lib install "Adafruit Fingerprint Sensor Library@2.1.3" "Adafruit SSD1306@2.5.13" "Adafruit GFX Library@1.11.11" "Adafruit BusIO@1.16.2"
arduino-cli compile --fqbn esp32:esp32:esp32 firmware/fingerprint_attendance
```

Open the `.ino` with `attendance.h` alongside it. Select your board's actual port and upload. Serial Monitor is 115200 baud with newline enabled.

## Sensor enrollment (separate maintenance operation)

The attendance firmware does not enroll, delete or download templates. Before testing it:

1. Verify exact sensor supply, UART logic voltage and baud rate from its datasheet. Use the proposed GPIO16/17 wiring only if compatible.
2. Open the Adafruit Fingerprint Sensor Library's `enroll` example. Adapt its non-SoftwareSerial transport to `HardwareSerial`/`Serial2` for ESP32. Create the fingerprint instance with `&Serial2`.
3. Before the example's `finger.begin(57600)`, call `Serial2.begin(57600, SERIAL_8N1, 16, 17)`. Retain the official example's enrollment/error checks. Verify that it compiles for your ESP32.
4. Enroll only consenting test participants into free IDs 1–127 that are within your sensor's actual capacity. Check existing IDs first; do not overwrite or erase templates casually.
5. Follow the example's two-capture prompt. Record only a private test-ID mapping locally. Re-upload the attendance sketch afterward.

Use the official library examples as the maintenance source; this repository does not include a tested enrollment sketch or claim a specific sensor capacity.

## First-time storage

The sketch calls `LittleFS.begin(false)` and will not auto-format on a mount failure. A fresh filesystem may therefore show FS ERROR.

Only after confirming there is no needed data (or after backing it up), type **INIT STORAGE** and send a newline. This explicitly formats the filesystem when it is unmounted; it can erase existing filesystem data. Press reset afterward. If mounted but the log is full or its header is invalid, this command is refused; export and inspect the data before a deliberate maintenance recovery. The command does not erase fingerprint templates or reset the Preferences session counter.

## Bench checks

Place an enrolled finger, confirm RECORDED and remove it. Scan it again: ALREADY should appear with no extra row in the same boot session. An unenrolled finger should show NO MATCH. A sensor/storage failure must not show RECORDED. Reset starts a NEW session, so the same ID may be recorded again. Missing OLED falls back to serial; display initialization failure is not a fingerprint error.

Send **EXPORT** to print the existing CSV between BEGIN_ATTENDANCE_CSV and END_ATTENDANCE_CSV. This is a local serial export. Save it privately; do not commit real records or identity mappings. Uploading a sketch normally need not erase the filesystem, but partition changes or erase-flash settings can destroy stored data. Back up before maintenance.

## Troubleshooting

| Symptom | Check |
|---|---|
| FP ERROR at boot | Sensor supply/current, TX/RX crossover, baud, protocol/password compatibility |
| OLED unavailable | SSD1306 vs SH1106, 0x3C/0x3D address, SDA/SCL, 3.3 V pull-ups |
| FS ERROR at first boot | Partition includes filesystem; deliberate initialization if appropriate |
| FS ERROR after operation | Export/inspect size, header, partial rows; no automatic format |
| ID ERROR | Enrolled ID must be 1–127; verify actual sensor capacity |
| No new row while finger stays down | Expected: finger removal required before next attempt |
| Duplicate after reboot | Expected: boot session changed; not daily deduplication |
| Brownout/reset during capture | Check peak sensor supply demand and grounding |
