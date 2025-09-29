# SL-Infinity Wireshark Capture Guide

This guide shows how to capture USB control transfer traffic from the SL-Infinity device on Windows to analyze the correct communication protocol.

## Why This Capture is Critical

The current Linux driver was failing because it was using **incorrect USB control transfer parameters**. The SL-Infinity device uses USB control transfers, but with specific parameters that differ from the original driver implementation.

The SL-Infinity device uses:
- **USB control transfers** (not HID reports)
- **Specific USB control transfer parameters** discovered through Wireshark capture
- **64-byte data packets** containing command data
- **Control endpoint 0x00** for all communication

## Capture Setup

### 1. Prerequisites
- Windows machine with L-Connect 3 installed
- SL-Infinity device connected and working
- Wireshark with USBPcap (run as administrator)
- Close all other RGB software to reduce noise

### 2. Device Identification
1. Open Device Manager (`Win + X` → Device Manager)
2. Look for **Universal Serial Bus controllers** → **LianLi Device** or similar
3. Right-click → Properties → Details → Hardware IDs
4. Look for: `USB\VID_0CF2&PID_A102&REV_0100&MI_00`
   - VID: `0x0CF2`
   - PID: `0xA102`
   - Interface: `MI_00` (this is the USB control interface)

### 3. Wireshark Configuration

#### Start Capture
1. Run Wireshark as **Administrator**
2. Select **USBPcap** interface
3. Apply filter: `usb.idVendor == 0x0CF2 && usb.idProduct == 0xA102`

#### What to Look For
The capture should show **USB control transfers** with specific parameters:

**Expected USB Control Transfer Format:**
```
USB Control Transfer (SET_REPORT)
├── bmRequestType: 0x21 (Host-to-device, Class, Interface)
├── bRequest: 0x09 (SET_REPORT)
├── wValue: 0x03e0 (Report ID 0x03, Feature Report)
├── wIndex: 0x01 (Interface 1)
├── wLength: 7 (data length)
└── Data: [7 bytes of command data starting with 0xE0]

USB Control Transfer (GET_REPORT)
├── bmRequestType: 0xa1 (Device-to-host, Class, Interface)
├── bRequest: 0x01 (GET_REPORT)
├── wValue: 0x01e0 (Report ID 0x01, Input Report)
├── wIndex: 0x01 (Interface 1)
├── wLength: 65 (expected data length)
└── Data: [65 bytes of status data from device]
```

**Correct Parameters (from successful capture):**
- **bmRequestType**: `0x21` (for SET_REPORT), `0xa1` (for GET_REPORT)
- **bRequest**: `0x09` (SET_REPORT), `0x01` (GET_REPORT)
- **wValue**: `0x03e0` (SET_REPORT), `0x01e0` (GET_REPORT)
- **wIndex**: `0x01` (Interface 1, not Interface 0)

### 4. Capture Process

#### Step 1: Start Capture
1. Start Wireshark capture
2. Open L-Connect 3
3. Wait for device to initialize

#### Step 2: Test Fan Control
1. Change fan speeds in L-Connect 3
2. Look for `URB_CONTROL out` messages with 64-byte data
3. Verify USB control transfer parameters match expected values

#### Step 3: Test RGB Control
1. Change RGB lighting modes
2. Look for USB control transfers with different command patterns
3. Note the command structure and data payload

#### Step 4: Test Mode Changes
1. Switch between different fan profiles
2. Look for `e0 50` commands (profile transition) in data payload
3. Note the complete command sequences

### 5. What to Capture

#### Critical Commands to Capture
1. **Device Initialization**
   - Initial USB control transfers when device connects
   - Any setup/initialization commands

2. **Fan Speed Control**
   - Commands: `e0 20`, `e0 21`, `e0 22`, `e0 23`
   - Data: `00 [speed_hex] 00 00...`
   - Look for `URB_CONTROL out` messages with correct parameters

3. **Profile Transitions**
   - Command: `e0 50`
   - Data: All zeros
   - Triggers mode changes

4. **RGB Control**
   - Commands: `e0 10-17` (control), `e0 30-37` (data)
   - Look for different patterns for different modes

5. **Status Monitoring**
   - Command: `e0 60`
   - Background status confirmation

### 6. Expected USB Control Transfer Structure

#### Device Initialization Command Example (from unplug/replug capture)
```
USB Control Transfer (SET_REPORT):
bmRequestType: 0x21
bRequest: 0x09 (SET_REPORT)
wValue: 0x03e0
wIndex: 0x01
wLength: 7
Data: e0 50 00 00 00 00 00

Response (SET_REPORT Response):
- Status: USBD_STATUS_SUCCESS
- Data Length: 0 (acknowledgment only)
- Time: ~0.001 seconds
```

#### Fan Speed Command Example (expected)
```
USB Control Transfer (SET_REPORT):
bmRequestType: 0x21
bRequest: 0x09 (SET_REPORT)
wValue: 0x03e0
wIndex: 0x01
wLength: 7
Data: e0 20 00 2e 00 00 00

Response (SET_REPORT Response):
- Status: USBD_STATUS_SUCCESS
- Data Length: 0 (acknowledgment only)
```

#### Status Request Example (from unplug/replug capture)
```
USB Control Transfer (GET_REPORT):
bmRequestType: 0xa1
bRequest: 0x01 (GET_REPORT)
wValue: 0x01e0
wIndex: 0x01
wLength: 65

Response (GET_REPORT Response):
- Status: USBD_STATUS_SUCCESS
- Data Length: 65 bytes
- Data: [65 bytes of device status]
```

### 7. Filtering and Analysis

#### Wireshark Filters
- **All USB control transfers**: `usb.transfer_type == 0x02`
- **SL-Infinity device**: `usb.idVendor == 0x0CF2 && usb.idProduct == 0xA102`
- **Device address**: `usb.device_address == 8` (from capture analysis)
- **Control transfers only**: `usb.urb_function == "URB_FUNCTION_CLASS_DEVICE"`

#### Key Things to Verify
1. **USB Control Transfer Parameters**: 
   - SET_REPORT: bmRequestType `0x21`, bRequest `0x09`, wValue `0x03e0`, wIndex `0x01`
   - GET_REPORT: bmRequestType `0xa1`, bRequest `0x01`, wValue `0x01e0`, wIndex `0x01`
2. **Data Length**: 
   - SET_REPORT: 7 bytes for commands
   - GET_REPORT: 65 bytes for status responses
3. **Command Format**: Should start with `e0` followed by command byte
4. **Transfer Type**: Should be `URB_CONTROL` (control transfers)
5. **Endpoint**: Should be `0x00` (control endpoint)
6. **Interface**: Should be Interface 1 (not Interface 0)

### 8. Common Issues

#### If You See USB Control Transfers
- This is CORRECT! The SL-Infinity uses USB control transfers
- Verify the parameters match the expected values (0x20, 0x00, 0x0000, 0x0000)
- Check if you have the right device (VID:PID 0x0CF2:0xA102)

#### If You See No USB Control Transfers
- Device might not be properly detected
- Check Device Manager for USB device (not HID device)
- Try different USB ports
- Ensure L-Connect 3 is running and communicating

#### If Control Transfers Have Wrong Parameters
- Check bmRequestType (should be 0x21 for SET_REPORT, 0xa1 for GET_REPORT)
- Check bRequest (should be 0x09 for SET_REPORT, 0x01 for GET_REPORT)
- Check wValue (should be 0x03e0 for SET_REPORT, 0x01e0 for GET_REPORT)
- Check wIndex (should be 0x01 for Interface 1, not 0x00)
- Verify data starts with 0xE0 command prefix

### 9. Expected Results

After successful capture, you should see:
- **USB control transfers** with correct parameters (0x21/0xa1, 0x09/0x01, 0x03e0/0x01e0, 0x01)
- **7-byte command packets** for SET_REPORT containing commands starting with 0xE0
- **65-byte status packets** for GET_REPORT responses
- **Command sequences** matching the protocol documentation
- **URB_CONTROL** messages for device communication
- **Interface 1** being used for all communication (not Interface 0)

### 10. Next Steps

Once you have the USB control transfer data:
1. Extract the exact USB control transfer parameters
2. Note the device address and endpoint
3. Document the command structure and data format
4. Use this data to fix the Linux driver

## Why This Will Fix the Driver

The original Linux driver was using incorrect parameters:
```c
usb_control_msg(dev, usb_sndctrlpipe(dev, 0x0), 0x21, 0x09, 0x02e0, 0x00, buffer, PACKET_SIZE, 100);
```

But it should be using the correct parameters from the capture:
```c
// For SET_REPORT (sending commands to device):
usb_control_msg(dev, usb_sndctrlpipe(dev, 0x0), 0x21, 0x09, 0x03e0, 0x01, buffer, 7, 100);

// For GET_REPORT (reading status from device):
usb_control_msg(dev, usb_sndctrlpipe(dev, 0x0), 0xa1, 0x01, 0x01e0, 0x01, buffer, 65, 100);
```

### Complete Fan Speed Control Implementation
```c
// Fan speed control function with correct parameters
int set_fan_speed(int port, int percentage) {
    if (port < 1 || port > 4) return -EINVAL;
    if (percentage < 0 || percentage > 100) return -EINVAL;
    
    uint8_t command[7] = {
        0xe0,                    // Command prefix
        0x20 + (port - 1),      // Port command (0x20-0x23)
        0x00,                   // Unknown parameter
        (uint8_t)percentage,    // Speed as hex
        0x00, 0x00, 0x00        // Padding
    };
    
    return usb_control_msg(dev, usb_sndctrlpipe(dev, 0x0), 0x21, 0x09, 0x03e0, 0x01, command, 7, 100);
}
```

This capture shows us the exact USB control transfer parameters that work with the SL-Infinity device.

## Device Initialization Sequence (from Unplug/Replug Capture)

When the SL-Infinity device is unplugged and replugged, the following initialization sequence occurs:

### 1. Device Enumeration (Packets 25-30)
- Standard USB device enumeration
- Device gets assigned address 8
- Interface 1 is configured for HID communication

### 2. Device Initialization Command (Packet 43)
```
SET_REPORT Request:
- bmRequestType: 0x21
- bRequest: 0x09 (SET_REPORT)
- wValue: 0x03e0
- wIndex: 0x01 (Interface 1)
- wLength: 7
- Data: e0 50 00 00 00 00 00
```

**Command Analysis:**
- `e0 50`: Profile transition command (device initialization)
- `00 00 00 00 00`: All zeros (no additional parameters)

### 3. Device Acknowledgment (Packet 44)
```
SET_REPORT Response:
- Status: USBD_STATUS_SUCCESS
- Data Length: 0 (acknowledgment only)
- Response Time: ~0.001 seconds
```

### 4. Status Request (Packet 45)
```
GET_REPORT Request:
- bmRequestType: 0xa1
- bRequest: 0x01 (GET_REPORT)
- wValue: 0x01e0
- wIndex: 0x01 (Interface 1)
- wLength: 65
```

### 5. Status Response (Packet 46)
```
GET_REPORT Response:
- Status: USBD_STATUS_SUCCESS
- Data Length: 65 bytes
- Data: [65 bytes of device status information]
```

### Key Findings from Initialization
1. **Interface 1 is used** for all communication (not Interface 0)
2. **Report IDs are different**: 0x03e0 for commands, 0x01e0 for status
3. **Command length is 7 bytes** (not 64 bytes as originally expected)
4. **Status response is 65 bytes** (not 64 bytes)
5. **Device responds quickly** (~0.001-0.002 seconds)
6. **e0 50 command** appears to be a device initialization/wake-up command

This initialization sequence shows that the device needs to be "woken up" with the `e0 50` command before it can accept other commands like fan speed control or RGB control.

## Fan Speed Control Commands (from Capture Analysis)

Based on the capture data from testing different fan speed profiles, the following command structure was confirmed:

### Command Structure
```
e0 [port] 00 [speed] 00 00 00
```

### Port Commands
- **`e0 20`**: Port 1 fan control
- **`e0 21`**: Port 2 fan control  
- **`e0 22`**: Port 3 fan control
- **`e0 23`**: Port 4 fan control

### Speed Values (from actual capture data)
- **`0x32`**: 50% speed (StdSP profile)
- **`0x3c`**: 60% speed (HighSP profile)
- **`0x64`**: 100% speed (FullSP profile)
- **`0x2c`**: 44% speed (Quiet profile)

### Example Commands Captured
```
e0 20 00 32 00 00 00  (Port 1: 50% speed - StdSP)
e0 20 00 3c 00 00 00  (Port 1: 60% speed - HighSP)
e0 20 00 64 00 00 00  (Port 1: 100% speed - FullSP)
e0 21 00 32 00 00 00  (Port 2: 50% speed - StdSP)
e0 22 00 64 00 00 00  (Port 3: 100% speed - FullSP)
e0 23 00 2c 00 00 00  (Port 4: 44% speed - Quiet)
```

### USB Control Transfer Parameters (Confirmed)
- **bmRequestType**: `0x21` (Host-to-device, Class, Interface)
- **bRequest**: `0x09` (SET_REPORT)
- **wValue**: `0x03e0` (Report ID 0x03, Feature Report)
- **wIndex**: `0x01` (Interface 1)
- **wLength**: `7` (data length)
- **Device Address**: `8` (not 4 as originally thought)

## Linux Driver Implementation Notes

### Critical Implementation Details

**1. Device Detection:**
```c
// USB device identification
#define LIAN_LI_VID    0x0CF2
#define LIAN_LI_PID    0xA102
#define DEVICE_INTERFACE 1  // NOT Interface 0!
```

**2. Command Buffer Management:**
```c
// Always use 7-byte buffers (not 64-byte!)
uint8_t command[7] = {0};
// Initialize with zeros to avoid garbage data
```

**3. USB Control Transfer Error Handling:**
```c
int result = usb_control_msg(dev, usb_sndctrlpipe(dev, 0x0), 0x21, 0x09, 0x03e0, 0x01, command, 7, 100);
if (result < 0) {
    dev_err(&dev->dev, "USB control transfer failed: %d\n", result);
    return result;
}
```

**4. Device Initialization Sequence:**
```c
// Always send e0 50 command first to wake up device
uint8_t init_cmd[7] = {0xe0, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00};
usb_control_msg(dev, usb_sndctrlpipe(dev, 0x0), 0x21, 0x09, 0x03e0, 0x01, init_cmd, 7, 100);
```

**5. Common Pitfalls to Avoid:**
- ❌ **Wrong Interface**: Don't use Interface 0, use Interface 1
- ❌ **Wrong Buffer Size**: Don't use 64-byte buffers, use 7-byte buffers
- ❌ **Wrong Device Address**: Device address is 8, not 4
- ❌ **Missing Initialization**: Always send `e0 50` command first
- ❌ **Wrong USB Parameters**: Use the exact parameters from capture analysis

**6. Testing Commands:**
```c
// Test fan speed control
set_fan_speed(1, 50);  // Port 1, 50% speed (StdSP)
set_fan_speed(1, 60);  // Port 1, 60% speed (HighSP)
set_fan_speed(1, 100); // Port 1, 100% speed (FullSP)
```

**7. Debugging Tips:**
- Use `usbmon` to monitor USB traffic: `sudo usbmon -i usbmon0`
- Check `/proc/bus/usb/devices` for device enumeration
- Use `lsusb -v` to verify device descriptors
- Monitor kernel logs: `dmesg | grep -i lian`
