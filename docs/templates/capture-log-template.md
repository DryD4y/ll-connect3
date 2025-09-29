# Capture Log Template

Fill one of these per device/firmware. Keep raw payloads and observations minimal but precise.

## Device
- Model:
- Hub/Controller:
- Firmware:
- Connection (USB path/bus/address):
- VID:PID:
- Date:
- Operator:

## Tools
- Windows: Wireshark x.y + USBPcap
- App: L-Connect 3 version
- Notes:

## Baseline
- App idle capture path:
- Observations:

## Actions
Repeat the block below per action.

### Action: <name>
- Start packet no:
- End packet no:
- pcapng path:
- summary.csv path:
- Notes:

Raw transfers:

```
Time, Dir, EP, Len, Bytes (hex), Meaning
```

Decoded fields (if known):

```
Field, Value, Notes
```

## Findings
- Command IDs:
- Checksums:
- Timing requirements:
- Feature flags/handshakes:
