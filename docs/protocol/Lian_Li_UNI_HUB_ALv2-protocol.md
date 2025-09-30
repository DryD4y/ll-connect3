# UNI HUB AL v2 Protocol Reference

This document contains the reverse-engineered USB protocol commands for the Lian Li UNI HUB AL v2 device, based on Windows capture analysis.

## Device Information
- **Model**: UNI HUB AL v2
- **Protocol**: USB HID
- **Command Format**: 64-byte packets starting with `e0`
- **Color Format**: RBG (Red, Blue, Green) - not RGB

## Fan Control Commands

### Set Fan Speed
```
e0 20(port) 00 (speed_hex)
```
- **Port**: 20-23 (ports 1-4)
- **Speed**: 
  - `0x64` = Max speed (~2000 RPM)
  - `0x0c` = Min speed (~250 RPM)

### Fan Speed Sync with Motherboard
**Enable sync:**
```
e0 10 62 11  (port 1)
e0 10 62 22  (port 2)
e0 10 62 44  (port 3)
e0 10 62 88  (port 4)
```

**Disable sync:**
```
e0 10 62 10  (port 1)
e0 10 62 20  (port 2)
e0 10 62 40  (port 3)
e0 10 62 80  (port 4)
```

## RGB Lighting Control

### Brightness Levels
| Percentage | Hex Value |
|------------|-----------|
| 0%         | 0x08      |
| 25%        | 0x03      |
| 50%        | 0x02      |
| 75%        | 0x01      |
| 100%       | 0x00      |

### Speed Levels
| Percentage | Hex Value |
|------------|-----------|
| 0%         | 0x02      |
| 25%        | 0x01      |
| 50%        | 0x00      |
| 75%        | 0xff      |
| 100%       | 0xfe      |

### Direction
- `0x01` = Backward
- `0x00` = Forward

### Set RGB Mode
```
e0 10 60 0X(port) 0X(fan_count)
e0 30(inner) + (2 * port) [RBG data]
e0 31(outer) + (2 * port) [RBG data]
```

**Color Data Format:**
- **Inner ring**: 8 RBG values per fan
- **Outer ring**: 12 RBG values per fan
- **Total space**: 116 bytes for inner, 138 bytes for outer

### Apply Settings to Ports
```
e0 11(outer) 04(mode) ff(speed) 0X(direction) 0X(brightness)
e0 10(inner) 04(mode) ff(speed) 0X(direction) 0X(brightness)
```

## Lighting Modes

| Mode | Name | Inner/Outer | Colors | Notes |
|------|------|-------------|--------|-------|
| 0x01 | Static Color | Both | 6 | |
| 0x02 | Breathing | Both | 6 | |
| 0x04 | Rainbow Morph | Both | 0 | |
| 0x05 | Rainbow | Either | 0 | |
| 0x06 | Breathing Rainbow | Outer | 0 | |
| 0x08 | Meteor Rainbow | Either | 0 | |
| 0x09 | Spinning | Outer | ? | |
| 0x0a | Spinning Rainbow | Outer | 0 | |
| 0x0b | Pulsating Rainbow | Outer | ? | |
| 0x0c | Rainbow Thing | Outer | 0 | |
| 0x11 | Flashes Blue | - | - | Returns to previous mode |
| 0x18 | Color Cycle | Inner | 4 | |
| 0x19 | Meteor | Both | 4 | |
| 0x1a | Runway | Both (no merge) | 2 | |
| 0x1b | Mop Up | Either | 2 | |
| 0x1c | Color Cycle | Outer | 4 | |
| 0x1d | Lottery | Either | 2 | |
| 0x1e | Wave | Either | 1 | |
| 0x1f | Spring | Either | 4 | |
| 0x20 | Tail Chasing | Either | 4 | |
| 0x21 | Warning | Either | 4 | |
| 0x22 | Voice | Either | 4 | |
| 0x23 | Mixing | Either | 2 | |
| 0x24 | Stack | Either | 2 | |
| 0x25 | Tide | Either | 4 | |
| 0x26 | Scan | Either | 1 | |
| 0x27 | Pac-Man | Inner | 2 | |
| 0x28 | Colorful City | Either | 0 | |
| 0x29 | Render | Either | 4 | |
| 0x2a | Twinkle | Either | 0 | |
| 0x2b | Rainbow | Both | 0 | |
| 0x2e | Color Cycle | Both | 4 | |
| 0x2f | Taichi | Both | 2 | |
| 0x30 | Warning | Both | 4 | |
| 0x31 | Voice | Both | 4 | |
| 0x32 | Mixing | Both (no merge) | 2 | |
| 0x33 | Tide | Both (no merge) | 4 | |
| 0x34 | Scan | Both (no merge) | 2 | |
| 0x35 | Contest | Both (no merge) | 3 | |
| 0x36 | Flashes Blue | - | - | Returns to different mode |
| 0x38 | Colorful City | Both | 0 | |
| 0x39 | Render | Both | 4 | |
| 0x3a | Twinkle | Both | 0 | |
| 0x3b | Wave | Both (no merge) | 1 | |
| 0x3c | Spring | Both (no merge) | 4 | |
| 0x3d | Tail Chasing | Both (no merge) | 4 | |
| 0x3e | Mop Up | Both (no merge) | 2 | |
| 0x3f | Tornado | Both | 4 | |
| 0x40 | Staggered | Both | 4 | |
| 0x41 | Spanning Teacups | Both | 4 | |
| 0x42 | Electric Current | Both (no merge) | 4 | |
| 0x43 | Stack | Both | 2 | |
| 0x44 | Scan | Both (merge) | 2 | |
| 0x45 | Contest | Both (merge) | 3 | |
| 0x46 | Runway | Both (merge) | 2 | |
| 0x47 | Mixing | Both (merge) | 2 | |
| 0x48 | Tide | Both (merge) | 4 | |
| 0x49 | Wave | Both (merge) | 1 | |
| 0x4a | Tail Chasing | Both (merge) | 4 | |
| 0x4b | Spring | Both (merge) | 4 | |
| 0x4c | Mop Up | Both (merge) | 2 | |
| 0x4f | Electric Current | Both (merge) | 4 | |

**⚠️ Warning**: Do not use modes above 0x4f as they break RGB functionality and require PSU power cycle to fix.

## Complete Command Examples

### Static Color Setup
```
60 10 04 01
e0 36(inner) [8 RBG values * fan_count]
e0 37(outer) [12 RBG values * fan_count]
e0 17 01 02
e0 16 01 02
```

### Color Cycle Setup
```
60 10 04 01
e0 36 [16 RBG values]
```

### Merge RGB Mode
```
e0 10 60 1 XX
e0 10 60 2 XX
e0 10 60 3 XX
e0 10 60 4 XX
e0 30 [RBG data]
e0 10 XX(mode) XX(speed) 0X(direction) 0X(brightness)
```

## Implementation Notes

- All commands use 64-byte USB HID packets
- Commands start with `e0` prefix
- Color data uses RBG format (Red, Blue, Green)
- Port numbers are 1-4, encoded as 0x20-0x23
- Fan count affects data packet size
- Some modes support "merge" functionality for synchronized effects

## Source
Based on reverse engineering analysis of L-Connect 3 USB traffic captures on Windows.
