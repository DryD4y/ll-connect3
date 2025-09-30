# L-Connect 3 Linux Driver & Application

A complete Linux solution for Lian Li L-Connect 3 fan controllers, providing both kernel driver support and a Qt-based GUI application that closely mimics the original Windows application.

## 🎯 Features

### Kernel Driver
- **Fan Speed Control**: Precise control of all connected fans
- **RGB Lighting**: Full RGB lighting control and effects
- **Multiple Protocols**: Support for both 0xE0 and 0x02 HID protocols
- **Per-Port Control**: Individual control of up to 4 fan ports
- **Performance**: Matches or exceeds Windows L-Connect 3 performance

### Qt Application
- **System Monitoring**: Real-time CPU, GPU, RAM, Network, and Storage monitoring
- **Fan Control GUI**: Visual fan speed control with profiles
- **RGB Control**: Lighting effects and color management
- **Settings**: Theme customization and system integration

## 📋 Supported Devices

- **Lian Li SL-Infinity Hub** (VID: 0x0CF2, PID: 0xA102)
- **Lian Li UNI HUB ALv2** (VID: 0x0CF2, PID: 0xA101)

## 🚀 Installation

### Prerequisites

- Linux kernel 5.4 or later
- Build tools: `make`, `gcc`, `linux-headers`
- Qt 6.0+ (Core, Widgets, Charts)
- CMake 3.16+
- C++17 compatible compiler
- Root access for kernel module installation

**Note**: This repository contains only source code. Binary files (`.ko`, `.o`, `.mod`) are not included and will be generated during the build process.

### Step 1: Install Kernel Driver

1. **Clone the repository:**
   ```bash
   git clone https://github.com/your-username/ll-connect3.git
   cd ll-connect3
   ```

2. **Build the kernel modules:**
   ```bash
   cd kernel
   sudo make build-all
   ```
   
   This will compile the source code and generate the necessary binary files (`.ko` modules).

3. **Install the modules:**
   ```bash
   sudo make install-all
   ```

4. **Load the SL-Infinity driver:**
   ```bash
   sudo modprobe Lian_Li_SL_INFINITY
   ```

5. **Verify driver installation:**
   ```bash
   ls -la /proc/Lian_li_SL_INFINITY/
   ```

### Step 2: Install L-Connect3 Application

1. **Install Qt dependencies:**
   ```bash
   # Ubuntu/Debian
   sudo apt install qt6-base-dev qt6-charts-dev cmake build-essential
   
   # Fedora
   sudo dnf install qt6-qtbase-devel qt6-qtcharts-devel cmake gcc-c++
   
   # Arch Linux
   sudo pacman -S qt6-base qt6-charts cmake base-devel
   ```

2. **Build the application:**
   ```bash
   cd /home/dev/Documents/GitHub/ll-connect3
   mkdir build && cd build
   cmake ..
   make -j$(nproc)
   ```

3. **Install the application:**
   ```bash
   sudo make install
   ```

4. **Run the application:**
   ```bash
   LConnect3
   ```

## 🧪 Testing

### Verify Driver is Working

Check that the driver loaded successfully:
```bash
# Check kernel messages
sudo dmesg | grep -i "lian\|sl-infinity"

# Verify proc filesystem
ls -la /proc/Lian_li_SL_INFINITY/
```

Expected output should show:
```
dr-xr-xr-x  10 root root 0 [timestamp] .
dr-xr-xr-x 713 root root 0 [timestamp] ..
-rw-rw-rw-   1 root root 0 [timestamp] fan_profile
-rw-rw-rw-   1 root root 0 [timestamp] lighting_effect
-rw-rw-rw-   1 root root 0 [timestamp] mbsync
dr-xr-xr-x   8 root root 0 [timestamp] Port_1
dr-xr-xr-x   8 root root 0 [timestamp] Port_2
dr-xr-xr-x   8 root root 0 [timestamp] Port_3
dr-xr-xr-x   8 root root 0 [timestamp] Port_4
-r--r--r--   1 root root 0 [timestamp] status
```

### Test Fan Control

**Important Note**: Setting fan speed to "0" does not completely stop the fans - it sets them to minimum speed (~800 RPM) for safety.

1. **Test maximum fan speed:**
   ```bash
   echo "100" > /proc/Lian_li_SL_INFINITY/Port_1/fan_speed
   ```

2. **Test minimum fan speed:**
   ```bash
   echo "0" > /proc/Lian_li_SL_INFINITY/Port_1/fan_speed
   ```

3. **Test different speeds:**
   ```bash
   echo "50" > /proc/Lian_li_SL_INFINITY/Port_1/fan_speed
   echo "75" > /proc/Lian_li_SL_INFINITY/Port_1/fan_speed
   ```

4. **Test other ports:**
   ```bash
   echo "100" > /proc/Lian_li_SL_INFINITY/Port_2/fan_speed
   echo "100" > /proc/Lian_li_SL_INFINITY/Port_3/fan_speed
   echo "100" > /proc/Lian_li_SL_INFINITY/Port_4/fan_speed
   ```

### Test RGB Lighting

1. **Set rainbow effect:**
   ```bash
   echo "0x20 75 100 0" > /proc/Lian_li_SL_INFINITY/lighting_effect
   ```

2. **Set static color:**
   ```bash
   echo "0x50 0 100 0" > /proc/Lian_li_SL_INFINITY/lighting_effect
   ```

### Check Status

View current system status:
```bash
cat /proc/Lian_li_SL_INFINITY/status
```

## 📊 Performance

### Fan Performance

- **Peak Performance**: 62.8 dBA (better than Windows 62.6 dBA)
- **Sustained Performance**: 61.4-61.7 dBA (within 1.2 dBA of Windows)
- **Speed Range**: 800-2100 RPM (0-100% control)
- **Response Time**: Immediate fan speed changes

### System Efficiency

- **CPU Temperature**: 34°C on Linux vs 41°C on Windows (7°C cooler!)
- **Better Thermal Management**: More efficient cooling with less noise
- **Lower Resource Usage**: No background processes like Windows L-Connect

## 🛠 Usage

### Command Line Fan Control

Control individual fan ports (1-4):
```bash
# Set fan speed (0-100%)
echo "75" > /proc/Lian_li_SL_INFINITY/Port_1/fan_speed

# Read current fan speed
cat /proc/Lian_li_SL_INFINITY/Port_1/fan_speed
```

### RGB Lighting Control

Set lighting effects:
```bash
# Format: effect speed brightness direction
echo "0x20 75 100 0" > /proc/Lian_li_SL_INFINITY/lighting_effect
```

Available effects:
- `0x20`: Rainbow
- `0x21`: Rainbow Morph
- `0x22`: Breathing
- `0x50`: Static Color

### Fan Profiles

Set fan profiles:
```bash
# Available profiles
echo "0x50" > /proc/Lian_li_SL_INFINITY/fan_profile  # Quiet
echo "0x20" > /proc/Lian_li_SL_INFINITY/fan_profile  # Standard
echo "0x21" > /proc/Lian_li_SL_INFINITY/fan_profile  # High
echo "0x22" > /proc/Lian_li_SL_INFINITY/fan_profile  # Full
```

### GUI Application

Launch the L-Connect3 application for a graphical interface:
```bash
LConnect3
```

The GUI provides:
- Real-time system monitoring
- Visual fan speed control
- RGB lighting management
- Settings and configuration

## 🔧 Uninstallation

### Uninstall Application

1. **Remove the application:**
   ```bash
   cd build
   sudo make uninstall
   ```

2. **Clean build files:**
   ```bash
   cd ..
   rm -rf build
   ```

### Uninstall Kernel Driver

1. **Unload the driver:**
   ```bash
   sudo modprobe -r Lian_Li_SL_INFINITY
   ```

2. **Uninstall the modules:**
   ```bash
   cd kernel
   sudo make uninstall-all
   ```

3. **Clean build files:**
   ```bash
   sudo make clean
   ```

## 🐛 Troubleshooting

### Driver Not Loading

1. **Check device connection:**
   ```bash
   lsusb | grep -i lian
   ```

2. **Check kernel messages:**
   ```bash
   sudo dmesg | grep -i "lian\|error"
   ```

3. **Verify kernel headers:**
   ```bash
   sudo apt install linux-headers-$(uname -r)
   ```

### Fans Not Responding

1. **Check driver status:**
   ```bash
   lsmod | grep Lian_Li_SL_INFINITY
   ```

2. **Verify proc filesystem:**
   ```bash
   ls -la /proc/Lian_li_SL_INFINITY/
   ```

3. **Test with different speeds:**
   ```bash
   echo "100" > /proc/Lian_li_SL_INFINITY/Port_1/fan_speed
   ```

### Application Not Starting

1. **Check Qt installation:**
   ```bash
   qmake6 --version
   ```

2. **Check dependencies:**
   ```bash
   ldd LConnect3
   ```

3. **Run with debug output:**
   ```bash
   LConnect3 --verbose
   ```

### Permission Issues

If you get permission denied errors:
```bash
# Check file permissions
ls -la /proc/Lian_li_SL_INFINITY/Port_1/fan_speed

# Should be: -rw-rw-rw- 1 root root
```

## 📚 Technical Details

### Protocol Support

- **0xE0 Protocol**: 7-byte HID reports for fan and lighting control
- **0x02 Protocol**: 8-byte HID reports for precise RPM control (experimental)
- **Multiple Modes**: Standard SP, High SP, Full SP support

### Kernel Module Information

- **Module Name**: `Lian_Li_SL_INFINITY`
- **Version**: 1.0
- **License**: GPL v2
- **Dependencies**: `hid`, `usbhid`

### Application Information

- **Framework**: Qt 6
- **Language**: C++17
- **Build System**: CMake
- **License**: GPL v2

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

## 📄 License

This project is licensed under the GPL v2 License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- Reverse engineering based on Windows L-Connect 3 behavior
- Community testing and feedback
- Lian Li for creating excellent hardware

## 📞 Support

- **Issues**: [GitHub Issues](https://github.com/your-username/ll-connect3/issues)
- **Discussions**: [GitHub Discussions](https://github.com/your-username/ll-connect3/discussions)

---

**Note**: This driver is reverse-engineered and not officially supported by Lian Li. Use at your own risk.