# Lian Li SL-INFINITY Fan Controller Protocol

**Device:** Lian Li SL-INFINITY Fan Hub (ENE 0cf2:a102)  
**Transport:** USB HID SET_REPORT  
**Report Length:** 7 bytes  
**Report ID:** Always `0xE0` (first byte)  

---

## Individual Port Control Protocol

**DISCOVERED:** Each fan port is controlled independently using a simple 7-byte HID SET_REPORT command.

### Command Format
```
[0] Report ID    (always 0xE0)
[1] Port Select  (0x20=Port1, 0x21=Port2, 0x22=Port3, 0x23=Port4)
[2] Reserved     (always 0x00)
[3] Duty Cycle   (0-100 decimal, percentage)
[4] Reserved     (always 0x00)
[5] Reserved     (always 0x00)
[6] Reserved     (always 0x00)
```

### Port Commands
- **Port 1:** `e0 20 00 <duty> 00 00 00`
- **Port 2:** `e0 21 00 <duty> 00 00 00`
- **Port 3:** `e0 22 00 <duty> 00 00 00`
- **Port 4:** `e0 23 00 <duty> 00 00 00`

Where `<duty>` is the percentage (0-100) directly.

### Usage Examples
```bash
# Set Port 1 to 50%
echo 50 > /proc/Lian_li_SL_INFINITY/Port_1/fan_speed

# Set Port 2 to 75%
echo 75 > /proc/Lian_li_SL_INFINITY/Port_2/fan_speed

# Set Port 3 to 25%
echo 25 > /proc/Lian_li_SL_INFINITY/Port_3/fan_speed

# Set Port 4 to 100%
echo 100 > /proc/Lian_li_SL_INFINITY/Port_4/fan_speed
```

---

## Key Features

- **Individual Control:** Each port controlled independently
- **Direct Percentage:** Input 0-100 directly as percentage
- **Simple Protocol:** Single 7-byte command per port
- **No Complex Sequences:** No port selection or multi-step commands needed
- **Real-time Control:** Immediate response to speed changes

---

## Implementation Notes

- **Kernel Driver:** Uses `/proc/Lian_li_SL_INFINITY/Port_X/fan_speed` interface
- **Input Validation:** Values clamped to 0-100 range
- **Error Handling:** Proper logging and error reporting
- **Debug Output:** Clear messages showing which port is being controlled

---

## Protocol Discovery Method

### Wireshark Analysis

The individual port control protocol was discovered by analyzing USB packet captures from the Windows L-Connect application using Wireshark/tshark.

#### Key Command
```bash
tshark -r port1.pcapng -Y "usb.bmRequestType == 0x21" -x
```

**What this does:**
- `-r port1.pcapng` - Read the packet capture file
- `-Y "usb.bmRequestType == 0x21"` - Filter for HID SET_REPORT commands
- `-x` - Display raw hex data

#### What to Look For
The command reveals HID SET_REPORT commands in this format:
```
USB Control (14 bytes): 0000  09 e0 03 01 00 07 00 e0 20 00 2f 00 00 00
```

**Key observations:**
- `0x21` = HID SET_REPORT request type
- `0xE0` = Report ID (first byte of payload)
- `0x20/21/22/23` = Port selector (second byte)
- `0x2F` = Duty cycle percentage (fourth byte)

#### Analysis Process
1. **Capture individual port changes** - Set one port at a time in L-Connect
2. **Filter for SET_REPORT commands** - Look for `usb.bmRequestType == 0x21`
3. **Extract payload patterns** - Focus on the 7-byte data after the USB header
4. **Compare across ports** - Identify the port selector byte differences
5. **Test in driver** - Implement the discovered protocol

#### Example Output
```
Frame (43 bytes): 0000  1c 00 10 d0 2a 48 8d 98 ff ff 00 00 00 00 1b 00
0010  00 04 00 03 00 00 02 0f 00 00 00 00 21 09 e0 03
0020  01 00 07 00 e0 20 00 2f 00 00 00
USB Control (14 bytes): 0000  09 e0 03 01 00 07 00 e0 20 00 2f 00 00 00
```

**Decoded:**
- `e0 20 00 2f 00 00 00` = Port 1 at 47% (0x2F = 47 decimal)

---

## Troubleshooting

### Common Issues
- **No response from fans:** Check USB connection and driver loading
- **All fans respond together:** Using wrong port selector byte
- **Permission denied:** Run with `sudo` or check `/proc` permissions
- **Driver not found:** Ensure kernel module is loaded (`lsmod | grep Lian_Li`)

### Debug Commands
```bash
# Check if driver is loaded
lsmod | grep Lian_Li

# Check proc entries exist
ls -la /proc/Lian_li_SL_INFINITY/

# View driver logs
dmesg | grep -i "sli\|lian" | tail -20

# Test individual ports
echo 50 > /proc/Lian_li_SL_INFINITY/Port_1/fan_speed
echo 75 > /proc/Lian_li_SL_INFINITY/Port_2/fan_speed
echo 25 > /proc/Lian_li_SL_INFINITY/Port_3/fan_speed
echo 100 > /proc/Lian_li_SL_INFINITY/Port_4/fan_speed
```