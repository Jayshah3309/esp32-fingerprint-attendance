# ESP32 Fingerprint Attendance System

An ESP32-WROOM-32 reference prototype for local fingerprint attendance with an SSD1306 OLED, UART fingerprint matching, per-session duplicate prevention and a flash CSV log.

**Status:** new reference implementation. The repository was originally empty; code and pin assignments are proposed, not recovered from the original hardware. Fingerprint model, OLED controller and actual physical wiring require confirmation. Hardware testing and real project media are pending.

![Proposed reference connections](hardware/circuit.svg)

## Problem and approach

Manual attendance recording can be repetitive and error-prone. This project demonstrates recording a matched fingerprint sensor ID and showing immediate local feedback. Fingerprint matching occurs inside a compatible sensor; the ESP32 stores the returned ID, confidence score, boot-session number and uptime. This does not establish identity assurance or measured recognition accuracy.

## Implemented reference features

- Hardware UART on GPIO16/17 for an Adafruit-library-compatible fingerprint sensor.
- 128×64 SSD1306 I²C OLED on GPIO21/22; serial fallback if unavailable.
- Match / no-match / duplicate / storage-error feedback.
- One attendance record per ID **per boot session**, IDs 1–127.
- Local LittleFS CSV append and explicit serial export.
- Persistent boot-session counter in Preferences.
- No automatic filesystem formatting, template download, names, networking or cloud upload.

## Working sequence

1. Verify display, filesystem and sensor communication.
2. Capture a finger, convert its image and search the sensor's enrolled templates.
3. Reject unknown IDs and duplicate matches in the current boot session.
4. Append a local CSV row; show RECORDED only after the write reports success.
5. Require finger removal before a new capture attempt.

**A reset starts a new attendance session.** This is not once-per-day attendance. CSV `uptime_ms` is time since boot and wraps after roughly 49.7 days; it is not a calendar timestamp. No NTP, RTC, timetable or check-in/check-out policy is included.

## Proposed hardware

| Component | Quantity | Requirement |
|---|---:|---|
| ESP32-WROOM-32 development board | 1 | Classic ESP32, GPIO16/17 available; no PSRAM conflict |
| UART fingerprint sensor | 1 | Adafruit protocol compatible, 57600 baud, 3.3 V-compatible UART |
| SSD1306 I²C OLED | 1 | 128×64, address 0x3C, 3.3 V-compatible module |
| Regulated sensor supply | As required | Exact voltage/current from sensor datasheet |
| USB cable, breadboard, wires | As needed | Shared GND |

Do not infer supply voltage or connector pin order from wire colours. Some fingerprint modules need a separate supply or UART level conversion. See [wiring](hardware/wiring.md).

## Files

- [Firmware](firmware/fingerprint_attendance/fingerprint_attendance.ino) and session logic
- [Wiring and power](hardware/wiring.md)
- [Installation, enrollment and recovery](docs/setup.md)
- [Architecture and data lifecycle](docs/architecture.md)
- [Test checklist and results status](docs/testing.md)
- [Blank measurements worksheet](docs/measurements.csv)
- [Data handling](docs/data-handling.md)
- [Limits and roadmap](docs/roadmap.md)
- [Contributors](docs/contributors.md) and [media instructions](media/README.md)

## Build and use

In Arduino IDE select **ESP32 Dev Module**, install the libraries below, open the sketch with `attendance.h` beside it, then upload to your actual serial port. Use Serial Monitor at **115200 baud** and follow [setup](docs/setup.md) for first-time filesystem initialization and sensor enrollment.

| Dependency | Pinned reference version |
|---|---|
| ESP32 board core | 3.0.7 |
| Adafruit Fingerprint Sensor Library | 2.1.3 |
| Adafruit SSD1306 | 2.5.13 |
| Adafruit GFX Library | 1.11.11 |
| Adafruit BusIO | 1.16.2 |

These are reproducible reference versions, not a claim that they are the newest. Wire, LittleFS and Preferences come with the ESP32 core. Exact CLI installation/build commands are in [setup](docs/setup.md) and the [CI workflow](.github/workflows/verify.yml).

Host tests:

```sh
g++ -std=c++11 -Wall -Wextra -Werror tests/attendance_test.cpp -o /tmp/attendance-test
/tmp/attendance-test
```

GitHub Actions runs host tests and a target ESP32 compile. These checks do not validate the physical sensor, display, matching performance or power-loss durability.

## Results and limitations

Physical recognition accuracy, scan time, supply stability and storage endurance have not been measured. The confidence value is sensor-specific, not a probability or published accuracy percentage. A successful match is not proof against spoofing or sharing. Flash storage is finite and not encrypted by this sketch. Power loss during a write can leave incomplete records. See [testing](docs/testing.md) and [data handling](docs/data-handling.md).

## Credits, license and references

Owner: [Jayshah3309](https://github.com/Jayshah3309). This reference package was prepared with AI assistance; original contributions and hardware evidence remain to be confirmed. License selection is pending; [LICENSE](LICENSE) is only a status notice.

- [Adafruit Fingerprint Sensor Library](https://github.com/adafruit/Adafruit-Fingerprint-Sensor-Library)
- [Adafruit SSD1306](https://github.com/adafruit/Adafruit_SSD1306)
- [Espressif UART API](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/serial.html)
- [Espressif Preferences API](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/preferences.html)
