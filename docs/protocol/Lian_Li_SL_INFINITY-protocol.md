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
