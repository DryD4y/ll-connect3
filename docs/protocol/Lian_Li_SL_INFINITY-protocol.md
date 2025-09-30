# 📝 L-Connect Fan Controller HID Protocol (Reverse-Engineered)

**Transport:** USB HID  
**Endpoint:** Interrupt OUT (Host → Device)  
**Report Length:** 8 bytes  
**Report ID:** Always `0x02` (first byte)  

---

## 📦 Packet Layout

```
[0] Report ID      (always 0x02)
[1-2] Reserved     (always 0x00 0x00)
[3-4] Control Code (fan duty step or profile selector)
[5]   Sub-mode     (used with some profiles)
[6-7] Padding      (always 0x00 0x00)
```

---

## 🌀 Manual Fan Duty (Fixed RPM Steps)

Payload format:

```
02 00 00 XX 00 00 00 00
```

Where `XX` = step index.  
You verified ~800 → 2100 RPM with 100 RPM increments.

| RPM Target | HID Data                  | Notes                  |
|------------|---------------------------|------------------------|
| 800 RPM    | `02 00 00 01 00 00 00 00` | Step 1 (lowest)        |
| 900 RPM    | `02 00 00 02 00 00 00 00` |                        |
| 1000 RPM   | `02 00 00 03 00 00 00 00` |                        |
| 1100 RPM   | `02 00 00 04 00 00 00 00` |                        |
| 1200 RPM   | `02 00 00 05 00 00 00 00` |                        |
| 1300 RPM   | `02 00 00 06 00 00 00 00` |                        |
| 1400 RPM   | `02 00 00 07 00 00 00 00` |                        |
| 1500 RPM   | `02 00 00 08 00 00 00 00` |                        |
| 1600 RPM   | `02 00 00 09 00 00 00 00` |                        |
| 1700 RPM   | `02 00 00 0A 00 00 00 00` |                        |
| 1800 RPM   | `02 00 00 0B 00 00 00 00` |                        |
| 1900 RPM   | `02 00 00 0C 00 00 00 00` |                        |
| 2000 RPM   | `02 00 00 0D 00 00 00 00` |                        |
| 2100 RPM   | `02 00 00 0E 00 00 00 00` | Step 14 (max tested)   |

---

## 🎛 Profiles & Modes

Special payloads (control bytes `FFxx`) switch global behavior:

| HID Data                  | Mode / Action          |
|----------------------------|------------------------|
| `02 00 00 FF FF 00 00 00` | Quiet Profile          |
| `02 00 00 FA FF 01 00 00` | Flat Profile           |
| `02 00 00 FD FF 02 00 00` | MB Sync (BIOS control) |
| `02 00 00 F9 FF 00 00 00` | Likely Performance     |
| `02 00 00 F8 FF FF 00 00` | Transition/alt state   |
| `02 00 00 F6 FF FF 00 00` | Transition/alt state   |

---

## 🛠 Suggested Driver API

```c
enum FanProfile {
    FAN_PROFILE_QUIET,
    FAN_PROFILE_FLAT,
    FAN_PROFILE_MB_SYNC,
    FAN_PROFILE_MANUAL
};

void setFanProfile(FanProfile profile);

void setFanRpm(int rpm); 
// rpm in range 800–2100, mapped to step 1–14
```

**Example implementation:**

```c
void setFanRpm(int rpm) {
    if (rpm < 800) rpm = 800;
    if (rpm > 2100) rpm = 2100;
    uint8_t step = (rpm - 800) / 100 + 1;

    uint8_t report[8] = {0x02,0x00,0x00,step,0x00,0x00,0x00,0x00};
    // send report to HID OUT endpoint
}
```

```c
void setFanProfile(FanProfile profile) {
    uint8_t report[8];
    switch(profile) {
        case FAN_PROFILE_QUIET:
            memcpy(report, (uint8_t[]){0x02,0x00,0x00,0xFF,0xFF,0x00,0x00,0x00}, 8);
            break;
        case FAN_PROFILE_FLAT:
            memcpy(report, (uint8_t[]){0x02,0x00,0x00,0xFA,0xFF,0x01,0x00,0x00}, 8);
            break;
        case FAN_PROFILE_MB_SYNC:
            memcpy(report, (uint8_t[]){0x02,0x00,0x00,0xFD,0xFF,0x02,0x00,0x00}, 8);
            break;
        case FAN_PROFILE_MANUAL:
            // handled by setFanRpm()
            return;
    }
    // send report to HID OUT endpoint
}
```

---

⚡ With this doc, you can now:
- Implement `setFanRpm()` to replace the buggy ramping logic.  
- Switch profiles programmatically (`Quiet`, `Flat`, `MB Sync`).  
- Extend later once we confirm what `F9FF/F8FF/F6FF` mean.

---

## 🔧 Alternative Protocol (0xE0 - 7-byte packets)

**Note:** During testing, we discovered the device also accepts 7-byte packets with Report ID `0xE0`:

```
[0] Report ID      (always 0xE0)
[1] Mode          (0x20=Standard SP, 0x21=High SP, 0x22=Full SP, 0x23=Unknown)
[2] Reserved      (always 0x00)
[3] Speed         (0x00-0xFF, percentage-based)
[4] Brightness    (0x00-0x64, for lighting)
[5] Direction     (0x00-0x01, for lighting)
[6] Reserved      (always 0x00)
```

**Mode Behavior:**
- `0x20` (Standard SP): Controls all fans together
- `0x21` (High SP): Controls individual fans (rear fan only in testing)
- `0x22` (Full SP): Unknown behavior (fans stopped in testing)
- `0x23` (Unknown): Unknown behavior (fans stuck in testing)

**Performance Results:**
- **Peak Performance**: 62.8 dBA (better than Windows 62.6 dBA)
- **Sustained Performance**: 61.4-61.7 dBA (within 1.2 dBA of Windows 62.6 dBA)
- **CPU Temperature**: 34°C on Linux vs 41°C on Windows (7°C cooler!)
- **Efficiency**: Better thermal management with comparable fan performance
- Individual fan control possible with `0x21` mode for future per-fan features

**Future Implementation Options:**
1. **Unified Control**: Use 0x02 protocol for precise RPM control of all fans
2. **Per-Fan Control**: Use 0xE0 protocol with mode 0x21 for individual fan control
3. **Hybrid Approach**: Use 0x02 for main control, 0xE0 for advanced features  
