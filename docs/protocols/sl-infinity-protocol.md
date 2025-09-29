# 🧩 Lian Li SL Infinity Hub Protocol — FINAL v2 (for Cursor)

Reverse-engineered from **Wireshark captures** of L-Connect 3 on Windows controlling the **Lian Li UNI HUB SL Infinity**.  
This unified document includes **Initialization**, **Fan Profiles**, and **Lighting Effects** with confirmed **Speed**, **Brightness**, and **Direction** bytes.

> **Device:** Lian Li SL Infinity Hub  
> **VID:PID:** `0x0CF2:0xA102`  
> **Interface:** 1 (HID Class)  
> **Report ID:** 0xE0 (Feature Reports)

---

## 🧭 Table of Contents
- [USB Device Overview](#-usb-device-overview)
- [Legend & Conventions](#-legend--conventions)
- [Initialization Sequence](#-initialization-sequence)
- [Fan Profiles](#-fan-profiles)
- [Lighting Effects](#-lighting-effects)
  - [Static, Rainbow, Rainbow Morph, Breathing](#static-rainbow-rainbow-morph-breathing)
  - [Meteor & Runway](#meteor--runway)
- [Linux HID Usage (hidraw examples)](#-linux-hid-usage-hidraw-examples)
- [Driver Outline (Cursor-ready hints)](#-driver-outline-cursor-ready-hints)
- [Notes & Open Items](#-notes--open-items)

---

## 📦 USB Device Overview

| Property | Value |
|-----------|-------|
| **VID:PID** | `0x0CF2:0xA102` |
| **Interface (HID)** | 1 |
| **Endpoints** | IN: 0x82 (Interrupt), OUT: Control only |
| **SET_REPORT Length** | 7 bytes |
| **GET_REPORT Length** | 65 bytes |
| **Report Type** | Feature (0x03) |

**Control Envelope (confirmed):**
- `bmRequestType = 0x21` (Host→Device, Class, Interface)
- `bRequest = 0x09` (**SET_REPORT**)
- `wValue = 0x03E0` (Feature, ReportID=E0)
- `wIndex = 1` (HID Interface)
- `wLength = 7`

**Status Polling (optional):**
- `bmRequestType = 0xA1` (Device→Host, Class, Interface)
- `bRequest = 0x01` (**GET_REPORT**)
- `wValue = 0x01E0`
- `wLength = 65`

---

## 🏷️ Legend & Conventions

**7-byte HID Feature Report Layout (0xE0):**

| Byte | Field | Description |
|------|--------|-------------|
| 0 | Report ID | Always `0xE0` |
| 1 | Mode | Profile / Effect selector |
| 2 | Reserved | Always `0x00` |
| 3 | Speed | `0x00–0x64` (0–100%), `0x2E` = 75% |
| 4 | Brightness | `0x00–0x64` (0–100%), `0x64` = 100% |
| 5 | Direction | `0x00` = Right, `0x01` = Left |
| 6 | Reserved | Always `0x00` |

> ⚙️ **Note:** Earlier captures showed only `0x00` in brightness; new data confirms active range up to `0x64`.  
> Both **Brightness** and **Direction** are independently encoded.

---

## 🔧 Initialization Sequence

| Step | Direction | ReportID | Payload (hex) | Description |
|------|------------|-----------|----------------|--------------|
| 1 | Host→Device | E0 | `e0 50 01 00 00 00 00` | Initial handshake |
| 2 | Host→Device | E0 | `e0 10 60 01 03 00 00` | Apply base configuration |
| 3 | Host←Device | E0 | (GET_REPORT, 65 bytes) | Status block |

**Driver Tip:** Send both `SET_REPORT` packets during initialization. Reading `GET_REPORT` is optional.

---

## 🌪️ Fan Profiles

| Profile | Payload (Hex) | Description |
|----------|----------------|--------------|
| Quiet | `e0 50 00 00 00 00 00` | Minimum curve |
| Standard SP | `e0 20 00 24 00 00 00` | Balanced curve |
| High SP | `e0 21 00 24 00 00 00` | Aggressive curve |
| Full SP | `e0 22 00 24 00 00 00` | Maximum performance |

---

## 🌈 Lighting Effects

All lighting commands use **ReportID 0xE0** (Feature).  
Default **Speed = 75% (0x2E)**, **Brightness = 100% (0x64)**, **Direction = Right (0x00)** unless specified.

### Static, Rainbow, Rainbow Morph, Breathing

| Effect | Payload (Hex) | Description |
|---------|----------------|--------------|
| Static Color (White) | `e0 50 00 00 64 00 00` | Static RGB (set separately) |
| Rainbow (Right) | `e0 20 00 2e 64 00 00` | Default Rainbow |
| Rainbow (Left) | `e0 20 00 2e 64 01 00` | Rainbow flipped Left |
| Rainbow Morph | `e0 21 00 2e 64 00 00` | Smooth Rainbow transition |
| Breathing | `e0 22 00 2e 64 00 00` | Multi-color fade |

### Meteor & Runway

| Effect | Payload (Hex) | Description |
|---------|----------------|--------------|
| Meteor | `e0 20 00 2e 64 00 00` | Meteor trail, multi-color |
| Runway | `e0 22 00 2e 64 00 00` | Two-color alternating effect |

> 💡 The `Mode` byte (`Byte 1`) determines base animation; palettes define visual colors.

---

## 🐧 Linux HID Usage (hidraw examples)

```c
#include <linux/hidraw.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int send_feature_report(const char *path, const unsigned char *report, size_t len) {
    int fd = open(path, O_RDWR | O_NONBLOCK);
    if (fd < 0) return -1;
    ioctl(fd, HIDIOCSFEATURE(len), report);
    close(fd);
    return 0;
}

int main() {
    const char *dev = "/dev/hidraw3";

    unsigned char init1[7] = {0xE0,0x50,0x01,0x00,0x00,0x00,0x00};
    unsigned char init2[7] = {0xE0,0x10,0x60,0x01,0x03,0x00,0x00};
    send_feature_report(dev, init1, 7);
    send_feature_report(dev, init2, 7);

    unsigned char rainbow_left[7] = {0xE0,0x20,0x00,0x2E,0x64,0x01,0x00};
    send_feature_report(dev, rainbow_left, 7);

    return 0;
}
```

---

## 🧱 Driver Outline (Cursor-ready hints)

**VID/PID Match Table**
```c
static const struct hid_device_id lianli_sl_infinity_table[] = {
    { HID_USB_DEVICE(0x0CF2, 0xA102) },
    { }
};
MODULE_DEVICE_TABLE(hid, lianli_sl_infinity_table);
```

**probe() steps**
1. Start HID
2. Send init1 / init2
3. Expose sysfs:
   - `/sys/class/leds/lianli:effect`
   - `/sys/class/hwmon/.../fan_profile`
4. Map names → payloads (see tables)

---

## 🧠 Notes & Open Items

- ✅ Confirmed: `Byte 4 = Brightness`, `Byte 5 = Direction`
- ⚙️ Brightness uses 0–100 scale (`0x00–0x64`)
- ⚙️ Direction uses `0x00` = Right, `0x01` = Left
- 🔜 Future: Capture color palette uploads (RGB triplets) for Static & Breathing modes

---

**Author:** Joey Troy  
**Date:** 2025-09-28  
