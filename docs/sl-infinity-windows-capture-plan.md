# SL-Infinity Windows Wireshark Capture Plan

## Objective
Capture detailed USB communication data from the Lian Li SL-Infinity device on Windows to understand the exact communication protocol and fix the Linux driver.

## Current Linux Driver Status
- ✅ Driver loads successfully
- ✅ Proc filesystem created (`/proc/Lian_li_SL_INFINITY/`)
- ✅ Command structure correct (32-byte packets, proper data format)
- ❌ All USB communication methods failing (-2 ENOENT, -22 EINVAL)
- ❌ Interface 1 appears to be claimed by HID subsystem

## Capture Plan

### Phase 1: Device Initialization Capture
**Goal:** Understand how the Windows driver initializes the device

**Steps:**
1. **Start Wireshark capture** with filter: `usb.transfer_type == 0x02 && usb.device_address == [DEVICE_ADDRESS]`
2. **Unplug and replug** the SL-Infinity device
3. **Wait for Windows driver to load** (watch for device recognition)
4. **Stop capture** after 30 seconds
5. **Save as:** `sl-infinity-initialization-[DATE].pcapng`

**What to look for:**
- Device enumeration process
- Interface claiming sequence
- Initialization commands sent to the device
- Any control transfers to Interface 0
- HID report setup

### Phase 2: Fan Speed Control Capture
**Goal:** Capture the exact USB transfers for fan speed commands

**Steps:**
1. **Start Wireshark capture** with same filter
2. **Open L-Connect 3** application
3. **Change fan speed** on Port 1 from 0% to 100% in steps:
   - 0% → 25% → 50% → 75% → 100%
   - Wait 2 seconds between each change
4. **Change fan speed** on other ports (Port 2, 3, 4)
5. **Stop capture** after all changes
6. **Save as:** `sl-infinity-fan-control-[DATE].pcapng`

**What to look for:**
- Exact USB transfer parameters (bmRequestType, bRequest, wValue, wIndex)
- Endpoint addresses being used
- Packet structure and data format
- Timing between commands
- Any handshaking or acknowledgment

### Phase 3: RGB Control Capture
**Goal:** Capture RGB lighting control commands

**Steps:**
1. **Start Wireshark capture** with same filter
2. **Change RGB colors** on different ports
3. **Try different lighting effects** (static, breathing, rainbow, etc.)
4. **Stop capture** after testing
5. **Save as:** `sl-infinity-rgb-control-[DATE].pcapng`

### Phase 4: Error Handling Capture
**Goal:** See what happens when commands fail

**Steps:**
1. **Start Wireshark capture** with same filter
2. **Disconnect and reconnect** the device while L-Connect is running
3. **Try to send commands** while device is disconnected
4. **Reconnect and try again**
5. **Stop capture**
6. **Save as:** `sl-infinity-error-handling-[DATE].pcapng`

## Analysis Checklist

### USB Transfer Analysis
- [ ] **Transfer Type:** Control, Interrupt, Bulk, or Isochronous?
- [ ] **Endpoint Address:** Which endpoint is actually used?
- [ ] **Interface Number:** Interface 0, 1, or both?
- [ ] **bmRequestType:** Exact value and meaning
- [ ] **bRequest:** Command type
- [ ] **wValue:** Parameter value
- [ ] **wIndex:** Interface/endpoint index
- [ ] **Data Length:** Actual packet size used
- [ ] **Data Format:** Exact byte structure

### Command Sequence Analysis
- [ ] **Initialization:** What commands are sent on device connect?
- [ ] **Fan Control:** Exact command structure for speed changes
- [ ] **RGB Control:** Command structure for lighting
- [ ] **Timing:** Delays between commands
- [ ] **Handshaking:** Any acknowledgment or response handling

### Device State Analysis
- [ ] **Interface Claiming:** How does Windows claim the interfaces?
- [ ] **Driver Loading:** What happens during driver initialization?
- [ ] **Error Recovery:** How does the driver handle communication errors?

## Expected Findings

### Likely Issues with Current Linux Driver
1. **Wrong Interface:** We're loading on Interface 0, but commands might need Interface 1
2. **Missing Initialization:** Windows might send setup commands we're missing
3. **Wrong Transfer Type:** We might need interrupt transfers instead of control transfers
4. **Interface Claiming:** We might need to properly claim Interface 1
5. **Timing Issues:** Commands might need specific delays or sequencing

### Key Questions to Answer
1. **Does Windows use Interface 0 or Interface 1 for commands?**
2. **What initialization sequence is required?**
3. **Are there any handshaking or acknowledgment requirements?**
4. **What is the exact packet structure for fan speed commands?**
5. **Are there any timing requirements between commands?**

## Deliverables

### Files to Create
1. `sl-infinity-initialization-[DATE].pcapng` - Device initialization capture
2. `sl-infinity-fan-control-[DATE].pcapng` - Fan speed control capture
3. `sl-infinity-rgb-control-[DATE].pcapng` - RGB control capture
4. `sl-infinity-error-handling-[DATE].pcapng` - Error handling capture

### Documentation to Create
1. **USB Protocol Analysis** - Detailed breakdown of captured commands
2. **Command Reference** - Exact parameters for each command type
3. **Initialization Sequence** - Step-by-step device setup
4. **Linux Driver Fixes** - Specific changes needed for the Linux driver

## Success Criteria
- [ ] Capture shows successful USB communication on Windows
- [ ] Identify exact USB parameters used for fan control
- [ ] Understand device initialization sequence
- [ ] Have enough data to fix the Linux driver
- [ ] Linux driver successfully controls fan speed

## Notes
- Use **Administrator privileges** for Wireshark
- **USBpcap** driver might be needed for USB capture
- **Device Manager** can help identify the correct device address
- **L-Connect 3** should be the latest version
- **Multiple captures** might be needed to get complete picture

## Next Steps After Capture
1. **Analyze captures** to identify working USB parameters
2. **Update Linux driver** with correct communication method
3. **Test Linux driver** with captured parameters
4. **Iterate** until fan control works successfully
