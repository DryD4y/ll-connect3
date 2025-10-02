
# Lian Li SL-INFINITY (ENE 0cf2:a102) — segmented HID feature driver

## Build & Install
```bash
# 1. Build the module
make

# 2. Copy it into the kernel module tree
sudo install -m 644 Lian_Li_SL_INFINITY.ko /lib/modules/$(uname -r)/extra/

# 3. Refresh module dependencies
sudo depmod -a

# 4. Remove any old version (ignore error if not loaded)
sudo rmmod Lian_Li_SL_INFINITY 2>/dev/null || true

# 5. Insert the new one
sudo modprobe Lian_Li_SL_INFINITY
# OR: sudo insmod ./Lian_Li_SL_INFINITY.ko

# 6. Check that /proc entries exist
ls -R /proc/Lian_li_SL_INFINITY
```

## Uninstall 
```bash
make clean
sudo make uninstall
```

## confirm proc entries exist
```bash
ls -la /proc/Lian_Li_SL_INFINITY
```

## Use
```bash
# Set per-port duty (0-100% directly)
echo 100 | sudo tee /proc/Lian_li_SL_INFINITY/Port_1/fan_speed
echo 100 | sudo tee /proc/Lian_li_SL_INFINITY/Port_2/fan_speed
echo 100 | sudo tee /proc/Lian_li_SL_INFINITY/Port_3/fan_speed
echo 100 | sudo tee /proc/Lian_li_SL_INFINITY/Port_4/fan_speed

# Test different speeds on each port
echo 30 | sudo tee /proc/Lian_li_SL_INFINITY/Port_1/fan_speed
echo 30 | sudo tee /proc/Lian_li_SL_INFINITY/Port_2/fan_speed
echo 30 | sudo tee /proc/Lian_li_SL_INFINITY/Port_3/fan_speed
echo 30 | sudo tee /proc/Lian_li_SL_INFINITY/Port_4/fan_speed

# Check logs for individual port control
sudo dmesg | grep -i "sli\|lian\|error\|fail" | tail -n 20
sudo dmesg | grep -E "Lian_Li_SL_INFINITY|Port.*set to" | tail -n 20

# Read back last-set duty
cat /proc/Lian_li_SL_INFINITY/Port_1/fan_speed
cat /proc/Lian_li_SL_INFINITY/Port_2/fan_speed
cat /proc/Lian_li_SL_INFINITY/Port_3/fan_speed
cat /proc/Lian_li_SL_INFINITY/Port_4/fan_speed

# Placeholder RPM (always 0 for now)
cat /proc/Lian_li_SL_INFINITY/Port_1/rpm
```

## What changed
- **Individual Port Control**: Each port now uses its own command byte for direct control
- **Direct Percentage Input**: No RPM conversion - input 0-100 directly as percentage
- **Simplified Protocol**: Single 7-byte HID SET_REPORT command per port
- **Per-Port Logging**: Clear debug messages showing which port is being controlled

## Protocol Details
**Individual Port Control Commands:**
```
Port 1: e0 20 00 <duty> 00 00 00
Port 2: e0 21 00 <duty> 00 00 00  
Port 3: e0 22 00 <duty> 00 00 00
Port 4: e0 23 00 <duty> 00 00 00
```
Where `<duty>` is the percentage (0-100) directly.

**Key Features:**
- No complex port selection sequence needed
- Each port controlled independently
- Direct percentage input (0-100)
- Real-time individual fan speed control

## Quick Install & Test
```bash
# Build and install
make
sudo install -m 644 Lian_Li_SL_INFINITY.ko /lib/modules/$(uname -r)/extra/
sudo depmod -a
sudo rmmod Lian_Li_SL_INFINITY 2>/dev/null || true
sudo modprobe Lian_Li_SL_INFINITY

# Test individual port control
echo 50 > /proc/Lian_li_SL_INFINITY/Port_1/fan_speed
echo 75 > /proc/Lian_li_SL_INFINITY/Port_2/fan_speed
echo 25 > /proc/Lian_li_SL_INFINITY/Port_3/fan_speed
echo 100 > /proc/Lian_li_SL_INFINITY/Port_4/fan_speed
```

