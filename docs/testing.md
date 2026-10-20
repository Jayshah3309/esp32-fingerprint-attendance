# Validation plan

## Evidence status

New reference implementation. Real sensor model, actual wiring, enrollment, OLED behaviour, supply stability and attendance measurements are unverified. The repository includes no actual participant records or biometrics.

Host tests exercise ID bounds, duplicates, committing only after acceptance, independent IDs, fresh sessions and timing rollover. GitHub Actions builds for the classic ESP32 target with pinned dependencies. Record the successful run link after execution. Build success does not establish recognition accuracy or hardware operation.

## Bench checklist

Record firmware commit, board/module, sensor model, OLED controller/address, supply, board-core/library versions, partition scheme, tester and date.

| ID | Procedure | Expected | Actual |
|---|---|---|---|
| A1 | Boot with verified sensor and initialized filesystem | READY and boot-session number | Not run |
| A2 | Scan enrolled test ID | Exactly one CSV row and RECORDED | Not run |
| A3 | Keep finger on sensor | No repeated capture record | Not run |
| A4 | Remove and rescan same ID | ALREADY; no additional row | Not run |
| A5 | Scan different enrolled ID | One separate record | Not run |
| A6 | Scan unenrolled finger | NO MATCH; no attendance row | Not run |
| A7 | Reset and rescan previous ID | New boot session permits new record | Not run |
| A8 | Disconnect sensor while unpowered, then boot | FP ERROR; no records accepted | Not run |
| A9 | OLED missing / wrong address | Serial fallback; no false OLED-success claim | Not run |
| A10 | Uninitialized filesystem | FS ERROR; no automatic erase | Not run |
| A11 | Near-full log or deliberate storage error in a disposable test image | No false RECORDED; inspect partial data | Not run |
| A12 | Export and compare record counts/content | Export matches local file | Not run |
| A13 | Repeated scans and sensor peak-load test | No brownout; record failures and timings | Not run |

Use `measurements.csv` with synthetic test identifiers. Keep real records private. Measure scan latency from presentation to confirmed recording with stated conditions. Report attempted scans, accepted matches, genuine rejections, impostor false accepts and communication/storage errors separately. Sensor confidence is not an accuracy percentage. Do not invent FAR/FRR or claim anti-spoofing from a few bench tests.

Power-loss experiments must use a disposable test dataset. Inspect incomplete rows; this prototype does not promise transaction durability. There is no actual classroom/production deployment evidence yet.
