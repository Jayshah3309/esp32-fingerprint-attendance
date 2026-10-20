# Data handling

The attendance sketch stores only sensor IDs, sensor confidence, boot session and uptime locally. IDs can still identify people if linked to a roster. Fingerprint templates remain in the sensor; no template/image export is implemented. This does not imply the sensor itself provides encryption or authenticated communication.

Use consenting participants and a clear purpose for enrollment. Keep any real-name mapping separate and private. Do not commit real attendance logs, fingerprint images/templates, serial captures, rosters or network credentials. The public measurement worksheet is blank; only synthetic examples should be published.

Local flash data and the serial export are not access-controlled or encrypted by the sketch. The prototype does not implement retention scheduling, secure deletion, cloud synchronization, admin roles or legal compliance guarantees. Before real use, determine who can enroll/export, how records are retained/corrected, and how participants can request removal through the sensor's documented maintenance tools. Avoid claiming tamper resistance or deployment readiness.

No actual biometric enrollment, deletion or export is performed by creating this repository. Those are separate physical-device operations.
