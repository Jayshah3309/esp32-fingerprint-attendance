# Limits and next work

## Present scope

Local UART fingerprint matching, SSD1306 feedback, flash CSV logging, ID-range checks and duplicate suppression per boot session. No online database, real names, RTC/NTP timestamp, daily attendance policy, enrollment UI or administrator authentication is provided.

## Next priorities

1. Confirm fingerprint model, voltage/current, UART level and protocol; compare reference pins with actual hardware.
2. Validate enrollment, OLED layout, first-time storage and duplicate handling on a bench.
3. Add actual prototype photos/video, confirmed contribution details and observed test results.
4. Measure recognition outcomes, scan latency, storage failure handling and peak supply current.
5. Add full-loop mocks/integration tests and a replay mode without treating replay as biometric validation.
6. Define a deliberate attendance session policy; add trusted RTC or network time only if required.
7. Engineer durable logging, recovery, export authorization and retention before real deployment.
8. Consider network synchronization only after defining offline queues, authenticated transport, duplicate keys and protection of credentials/data.

A simulator may illustrate OLED/UI and synthetic matches; it cannot validate a real fingerprint sensor's accuracy. Do not publish a release as hardware-verified until evidence exists. GPS, remote notifications and dashboards are not current features.
