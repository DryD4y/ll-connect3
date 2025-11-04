# <img src="resources/logo.png" width="50" align="center"/> LL-Connect 3 — SL-Infinity Hub

Complete Linux support for the Lian Li SL‑Infinity hub: a kernel fan driver and a Qt desktop app that mirrors Windows L‑Connect 3. The installer has been tested and verified on Kubutu 24.04LTS

## Support This Project

If you find this project helpful, please consider supporting its development:

[![PayPal](https://img.shields.io/badge/Donate-PayPal-blue.svg)](https://www.paypal.com/paypalme/joeytroynm)

Your support helps maintain and improve this open-source driver for the Lian Li community!

## Quick Install (Recommended)

Use the provided scripts to install everything (libraries + driver + app) automatically.

```bash
git clone https://github.com/joeytroy/ll-connect3.git
cd ll-connect3
./install.sh
```

After install:
- Run the app: `LLConnect3`
- The kernel module auto‑loads on boot (`Lian_Li_SL_INFINITY`)
- Fan control is available at `/proc/Lian_li_SL_INFINITY/Port_X/fan_speed`

Optional GPU monitoring tools (install based on your GPU):

```bash
# NVIDIA
nvidia-smi   # included with NVIDIA proprietary drivers

# AMD
sudo apt install radeontop

# Intel
sudo apt install intel-gpu-tools
```

### Uninstall

```bash
cd ll-connect3
./uninstall.sh
```

The uninstall script removes the app, the kernel module, its auto‑load config, and build artifacts.

## Application Overview

- System Info shows CPU, GPU, RAM, Network and Hard Drive useage

<img src="docs/screenshots/systeminfo.png" width="600"/>

- Per‑port custom fan curves with 4 presets and 3 custom slots. Each port can have it's own curve

<img src="docs/screenshots/fanprofile.png" width="600"/>

- Built‑in RGB page with 14 lighting effects: Breathing, Groove, Meteor, Mixing, Neon, Rainbow Wave, Runway, Spectrum Cycle, Stack, Staggered, Static, Tide, Tunnel, and Voice. Each effect supports color, speed/brightness control, with direction control where applicable 

<img src="docs/screenshots/lighting.png" width="600"/>

- Basic Settings page with debug capability along with reseting application settings to defaults

<img src="docs/screenshots/settings.png" width="600"/>

## Development: Manual Build Instructions

If you prefer manual steps or are contributing, follow this section. See [CONTRIBUTING.md](CONTRIBUTING.md) for detailed contribution guidelines, and [kernel/INSTALL.md](kernel/INSTALL.md) for comprehensive kernel driver documentation.

### Prerequisites

- Linux kernel 5.4+
- Build tools and headers: `make`, `gcc`, `linux-headers` (your running kernel)
- Qt 6 (Core, Widgets), CMake 3.16+, `pkg-config`
- Libraries: `lm-sensors`, `libusb-1.0-0-dev`, `libhidapi-dev`

### Kernel Driver (Fan Only)

```bash
cd ll-connect3/kernel
make

# Install to system and refresh deps
make install    # copies .ko into /lib/modules/$(uname -r)/extra and runs depmod

# Load the module
sudo rmmod Lian_Li_SL_INFINITY 2>/dev/null || true
sudo modprobe Lian_Li_SL_INFINITY

# Auto‑load on boot
echo "Lian_Li_SL_INFINITY" | sudo tee /etc/modules-load.d/lian-li-sl-infinity.conf

# Verify
lsmod | grep Lian_Li
ls -R /proc/Lian_li_SL_INFINITY
```

Notes:
- Write 0–100 to `/proc/Lian_li_SL_INFINITY/Port_X/fan_speed` to set per‑port speed.
- Use the app for persistent fan presence configuration.

### Qt Application

```bash
cd ll-connect3
mkdir -p build && cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr ..
make -j$(nproc)
sudo make install

# Run the app
LLConnect3

# To uninstall the application
sudo make uninstall
```

### Testing

After building/installing manually, use these quick checks:

```bash
# Verify driver is loaded and proc entries exist
lsmod | grep Lian_Li
ls -R /proc/Lian_li_SL_INFINITY

# Per‑port fan speed test (0–100)
echo 100 | sudo tee /proc/Lian_li_SL_INFINITY/Port_1/fan_speed
echo  50 | sudo tee /proc/Lian_li_SL_INFINITY/Port_2/fan_speed

# Read back values
cat /proc/Lian_li_SL_INFINITY/Port_1/fan_speed
cat /proc/Lian_li_SL_INFINITY/Port_2/fan_speed

# Kernel logs (helpful for debugging)
sudo dmesg | grep -i "sli" | tail -20
```

Troubleshooting tips:
- Make sure kernel headers for your running kernel are installed.
- If you rebuilt the module, `sudo rmmod Lian_Li_SL_INFINITY && sudo modprobe Lian_Li_SL_INFINITY`.
- After distro kernel updates, rebuild: `make clean && make && make install` in `kernel/`.


## Troubleshooting (Basics)

### Secure Boot Compatibility

**IMPORTANT:** This driver does not work with Secure Boot enabled, as the kernel module is unsigned.

To check if Secure Boot is enabled:
```bash
mokutil --sb-state
```

If Secure Boot is enabled, you have two options:

**Option 1: Disable Secure Boot (Recommended for ease)**
1. Reboot and enter your BIOS/UEFI settings (usually F2, F10, F12, or Del during boot)
2. Find the Secure Boot option (usually under Security or Boot settings)
3. Disable Secure Boot
4. Save and exit

**Option 2: Sign the kernel module (Advanced)**
```bash
# Generate signing keys (one-time setup)
sudo /usr/src/linux-headers-$(uname -r)/scripts/sign-file sha256 \
    /path/to/private/key.priv /path/to/public/key.der \
    kernel/Lian_Li_SL_INFINITY.ko

# Then enroll the key with MOK (Machine Owner Key)
sudo mokutil --import /path/to/public/key.der
```

For more details on module signing, see the [kernel documentation](https://www.kernel.org/doc/html/latest/admin-guide/module-signing.html).

### Common Issues

```bash
# Detect device
lsusb | grep -i lian

# Kernel logs
sudo dmesg | grep -i "sli" | tail -20

# Missing headers (Debian/Ubuntu)
sudo apt install linux-headers-$(uname -r)

# Check if Secure Boot is blocking the module
sudo dmesg | grep -i "lockdown\|secure"
```

## Protocol Documentation

For detailed information about the fan control protocol, including USB HID commands and reverse-engineering process:

- [Lian Li SL-INFINITY Protocol Documentation](docs/protocol/Lian_Li_SL_INFINITY-protocol.md)

## Supported Device

- Lian Li SL‑Infinity Hub (VID: 0x0CF2, PID: 0xA102)

## License

GPL v2 — see `LICENSE`.

---

### Overview
LL-Connect3 is a fully self-contained, open-source fan and RGB controller for the Lian Li SL-Infinity Hub and compatible fans.  
It includes integrated RGB control logic adapted from the OpenRGB project — you do **not** need to install OpenRGB separately.

### Credits
- Portions of the RGB control code are derived from the [OpenRGB](https://openrgb.org/) project and are used under the terms of the GNU General Public License (GPL).  
- The majority of the implementation was authored with assistance from Cursor AI, while I focused on debugging, testing, and reverse-engineering the hardware protocols.  
- All modifications and integration work are © 2025 Joey Troy and released under the same license.
