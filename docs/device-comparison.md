# L-Connect Device Comparison

## Overview
This document compares the capabilities of the two supported Lian Li devices and their respective drivers.

## Device Specifications

### SL-Infinity Hub
- **VID:PID**: `0x0cf2:0xa102`
- **Protocol**: HID Feature Reports (7-byte packets)
- **Driver**: `Lian_Li_SL_INFINITY.c`
- **Complexity**: Simple

### UNI HUB ALv2
- **VID:PID**: `0x0cf2:0xa104`
- **Protocol**: USB Control Transfers (353-byte packets)
- **Driver**: `Lian_Li_UNI_HUB_ALv2.c`
- **Complexity**: Advanced

## Feature Comparison

### Lighting Effects

#### SL-Infinity (6 effects)
| Effect | Mode | Description |
|--------|------|-------------|
| Static Color | 0x50 | Fixed color display |
| Rainbow | 0x20 | Rainbow wave effect |
| Rainbow Morph | 0x21 | Morphing rainbow |
| Breathing | 0x22 | Breathing effect |
| Meteor | 0x20 | Meteor shower effect |
| Runway | 0x22 | Runway lighting |

#### UNI HUB ALv2 (46 effects)
| Effect | Mode | Flags | Description |
|--------|------|-------|-------------|
| Static Color | 0x01 | INNER \| OUTER \| INNER_AND_OUTER \| NOT_MOVING \| BRIGHTNESS | Fixed color display |
| Breathing | 0x02 | INNER \| OUTER \| INNER_AND_OUTER \| NOT_MOVING \| BRIGHTNESS \| SPEED | Breathing effect |
| Rainbow Morph | 0x04 | INNER \| OUTER \| INNER_AND_OUTER \| BRIGHTNESS \| SPEED | Morphing rainbow |
| Rainbow | 0x05 | INNER \| OUTER \| BRIGHTNESS \| SPEED \| DIRECTION | Rainbow wave |
| Breathing Rainbow | 0x06 | OUTER \| BRIGHTNESS \| SPEED | Breathing rainbow |
| Meteor Rainbow | 0x08 | INNER \| OUTER \| BRIGHTNESS \| SPEED | Meteor rainbow |
| Color Cycle | 0x18 | INNER \| BRIGHTNESS \| SPEED \| DIRECTION | Color cycling |
| Meteor | 0x19 | INNER \| OUTER \| INNER_AND_OUTER \| BRIGHTNESS \| SPEED \| DIRECTION | Meteor shower |
| Runway | 0x1a | INNER \| OUTER \| INNER_AND_OUTER \| MERGE \| BRIGHTNESS \| SPEED \| DIRECTION | Runway lighting |
| Mop Up | 0x1b | INNER \| OUTER \| SPEED \| BRIGHTNESS | Mop up effect |
| ... | ... | ... | ... |
| Stack | 0x43 | INNER_AND_OUTER \| BRIGHTNESS \| SPEED \| DIRECTION | Stack effect |

**Total: 46 different lighting effects**

### Fan Control

#### SL-Infinity (4 profiles)
| Profile | Mode | Speed | Description |
|---------|------|-------|-------------|
| Quiet | 0x50 | 0% | Minimum speed |
| Standard | 0x20 | 36% | Standard speed |
| High | 0x21 | 36% | High speed |
| Full | 0x22 | 36% | Full speed |

#### UNI HUB ALv2 (Advanced curves)
- **Temperature-based fan curves** with multiple points
- **Individual port control** (4 ports)
- **Real-time temperature monitoring**
- **Custom curve editing**
- **Preset management**

### RGB Control

#### SL-Infinity
- **Basic RGB control**
- **Single ring control**
- **Simple color selection**

#### UNI HUB ALv2
- **Separate inner/outer ring control**
- **Individual port RGB control** (4 ports)
- **Color palette support** (48 inner colors, 72 outer colors)
- **Merge mode** for synchronized effects
- **Advanced color picker**

### Advanced Features

#### SL-Infinity
- **Basic fan speed control**
- **Simple lighting effects**
- **Safety mode** (limits fan speeds)

#### UNI HUB ALv2
- **Motherboard sync** (temperature-based control)
- **Fan curve editor** with visual interface
- **Temperature monitoring** (CPU temperature)
- **Preset management** (save/load configurations)
- **Individual port configuration**
- **Advanced RGB modes** (46 different effects)
- **Color picker** with palette support
- **Merge mode** for complex lighting

## Protocol Differences

### SL-Infinity (HID)
```c
// 7-byte HID Feature Report
report[0] = 0xE0;     // Report ID
report[1] = mode;     // Mode (Profile/Effect)
report[2] = 0x00;     // Reserved
report[3] = speed;    // Speed (0x00-0x64)
report[4] = brightness; // Brightness (0x00-0x64)
report[5] = direction; // Direction (0x00=Right, 0x01=Left)
report[6] = 0x00;     // Reserved
```

### UNI HUB ALv2 (USB)
```c
// 353-byte USB Control Transfer
buffer[0] = 0xe0;     // Header
buffer[1] = 0x60;     // Command type
buffer[2] = 0x10;     // Sub-command
buffer[3] = port;     // Port number
buffer[4] = fan_count; // Fan count
// ... 349 more bytes for color data, modes, etc.
```

## UI Implementation

### Unified Interface
The `ui_unified.c` provides a single interface that adapts based on the detected device:

- **SL-Infinity**: Shows 6 lighting effects in a simple grid
- **UNI HUB ALv2**: Shows 46 lighting effects with advanced controls
- **Auto-detection**: Automatically switches UI based on connected device
- **Feature-appropriate controls**: Only shows relevant controls for each device

### Device Detection
```c
// Check for SL-Infinity
if (access("/proc/Lian_li_SL_INFINITY", F_OK) == 0) {
    current_device = DEVICE_SL_INFINITY;
}
// Check for UNI HUB ALv2
else if (access("/proc/Lian_li_UNI_HUB_ALv2", F_OK) == 0) {
    current_device = DEVICE_UNI_HUB;
}
```

## Summary

The **UNI HUB ALv2** is significantly more advanced than the **SL-Infinity**, offering:

- **7.7x more lighting effects** (46 vs 6)
- **Advanced fan control** with temperature curves
- **Individual port control** for both fans and RGB
- **Complex RGB modes** with inner/outer ring separation
- **Professional features** like motherboard sync and preset management

The **SL-Infinity** provides:

- **Simple, easy-to-use interface**
- **Basic fan profiles** for quick setup
- **Essential lighting effects** for most users
- **Reliable HID-based communication**

Both devices are fully supported by the unified L-Connect application, which automatically adapts its interface to show the appropriate features for each device.
