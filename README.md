# L-Connect 3 Linux Port

A Qt-based Linux port of L-Connect 3 for Lian Li hardware, featuring a modern dark theme interface that closely mimics the original Windows application.

## Features

### System Monitoring
- **Real-time System Resource Monitoring**: CPU, GPU, RAM, Network, and Storage monitoring
- **CPU Monitoring**: Load percentage, temperature, clock rate, power consumption, and voltage
- **GPU Monitoring**: Load percentage, temperature, clock rate, power consumption, and memory usage
- **Network Monitoring**: Real-time upload/download speeds with automatic interface detection
- **RAM Monitoring**: Usage percentage and memory details with circular progress indicator
- **Storage Monitoring**: Filesystem usage and available space

### Fan & Pump Control
- **Fan Profile Management**: Basic fan control profiles (work in progress)
- **Port Configuration**: Support for multiple fan/pump ports (work in progress)

### RGB Lighting Control
- **Lighting Effects**: RGB lighting control for Lian Li devices (work in progress)
- **Speed & Brightness Control**: Adjustable lighting parameters (work in progress)

### Settings & Configuration
- **Theme Customization**: Dark theme with L-Connect 3 styling
- **System Integration**: Linux-native hardware monitoring

## Screenshots

<img src="docs/img/systeminfo.png" alt="L-Connect 3 Linux Application" width="500">

The application features a modern dark theme interface with:
- Left sidebar navigation with icons
- System Info page with real-time monitoring cards
- CPU monitoring with circular progress indicator and detailed metrics
- GPU monitoring with temperature, clock rate, power, and memory usage
- Network monitoring with real-time upload/download speeds
- RAM monitoring with usage percentage and memory details
- Storage monitoring with filesystem usage information

## Requirements

- **Qt 6.0+** (Core, Widgets, Charts) - *Not included by default in most Linux distributions*
- **CMake 3.16+**
- **C++17 compatible compiler**
- **Linux distribution** (Ubuntu 20.04+, Fedora 33+, Arch Linux, etc.)

## Dependencies

**Note**: Qt is not included by default in most Linux distributions, but it's easily available through package managers. The installation is typically just one command.

### Quick Installation

```bash
# Ubuntu/Debian (Most Common)
sudo apt update
sudo apt install qt6-base-dev qt6-charts-dev cmake build-essential lm-sensors

# Fedora/RHEL/CentOS
sudo dnf install qt6-qtbase-devel qt6-qtcharts-devel cmake gcc-c++ lm-sensors

# Arch Linux
sudo pacman -S qt6-base qt6-charts cmake gcc lm-sensors

# openSUSE
sudo zypper install qt6-base-devel qt6-qtcharts-devel cmake gcc-c++ lm-sensors

# Gentoo
sudo emerge -av qtbase qtcharts cmake lm-sensors
```

### GPU Monitoring Tools (Optional)

For enhanced GPU monitoring capabilities, install the appropriate tools for your graphics card:

**Note**: NVIDIA monitoring requires the proprietary NVIDIA driver. The open-source `nouveau` driver has limited monitoring capabilities.

```bash
# NVIDIA GPU monitoring
sudo apt install nvidia-utils-535        # Ubuntu/Debian
sudo dnf install nvidia-utils            # Fedora/RHEL/CentOS
sudo pacman -S nvidia-utils              # Arch Linux

# AMD GPU monitoring
sudo apt install radeontop               # Ubuntu/Debian
sudo dnf install radeontop               # Fedora/RHEL/CentOS
sudo pacman -S radeontop                 # Arch Linux

# Intel GPU monitoring
sudo apt install intel-gpu-tools         # Ubuntu/Debian
sudo dnf install intel-gpu-tools         # Fedora/RHEL/CentOS
sudo pacman -S intel-gpu-tools           # Arch Linux
```

### Why Qt?

Qt is the best choice for this application because:
- **Professional GUI capabilities** - Perfect for complex desktop applications
- **Cross-platform support** - Works on Linux, Windows, and macOS
- **Excellent documentation** - Comprehensive guides and examples
- **Easy installation** - Available in all major Linux package managers
- **Active community** - Large ecosystem and support

### Why lm-sensors?

The `lm-sensors` package is essential for system monitoring because:
- **CPU Temperature Monitoring** - Provides accurate temperature readings from hardware sensors
- **Power Consumption** - Enables power monitoring through RAPL (Running Average Power Limit)
- **Voltage Detection** - Allows voltage monitoring for CPU and other components
- **Hardware Compatibility** - Works with Intel, AMD, and other CPU manufacturers
- **Real-time Data** - Provides live system metrics for the monitoring dashboard

### Alternative Installation Methods

If you prefer not to use package managers:

1. **Qt Installer**: Download from [qt.io](https://www.qt.io/download-qt-installer)
2. **Conan package manager**: `conan install qt/6.5.0@`
3. **vcpkg**: `vcpkg install qt6-base qt6-charts`

### Troubleshooting

**"Qt not found" error?**
- Make sure you installed the **development packages** (ending with `-dev` or `-devel`)
- Verify installation: `pkg-config --modversion Qt6Core`
- Check CMake can find Qt: `cmake --find-package -DNAME=Qt6 -DCOMPILER_ID=GNU -DLANGUAGE=CXX`

**System monitoring showing "N/A" values?**
- Install lm-sensors: `sudo apt install lm-sensors` (Ubuntu/Debian)
- Run sensor detection: `sudo sensors-detect --auto`
- Check available sensors: `sensors`
- For power monitoring, some features require root permissions

**GPU monitoring showing "--" values?**
- **NVIDIA**: Install proprietary driver: `sudo apt install nvidia-driver-535`
- **AMD**: Install radeontop: `sudo apt install radeontop`
- **Intel**: Install intel-gpu-tools: `sudo apt install intel-gpu-tools`
- Check current driver: `lspci -k | grep -A 2 -i vga`
- Note: Open-source drivers (nouveau, radeon) have limited monitoring capabilities

## Building

1. **Clone the repository**:
   ```bash
   git clone https://github.com/your-username/ll-connect.git
   cd ll-connect
   ```

2. **Create build directory**:
   ```bash
   mkdir build && cd build
   ```

3. **Configure with CMake**:
   ```bash
   cmake ..
   ```

4. **Build the application**:
   ```bash
   make -j$(nproc)
   ```

5. **Run the application**:
   ```bash
   ./LConnect3
   ```

## Project Structure

```
ll-connect/
├── src/
│   ├── main.cpp                 # Application entry point
│   ├── mainwindow.h/.cpp        # Main window implementation
│   ├── pages/                   # Individual page implementations
│   │   ├── systeminfopage.h/.cpp
│   │   ├── fanprofilepage.h/.cpp
│   │   ├── lightingpage.h/.cpp
│   │   ├── slinfinitypage.h/.cpp
│   │   └── settingspage.h/.cpp
│   └── widgets/                 # Custom widget implementations
│       ├── monitoringcard.h/.cpp
│       ├── customslider.h/.cpp
│       └── fanwidget.h/.cpp
├── CMakeLists.txt              # CMake build configuration
└── README.md                   # This file
```

## Features Implementation

### System Monitoring
- **MonitoringCard**: Custom widget for displaying system metrics
- **Circular Progress**: Animated circular progress indicators
- **Real-time Updates**: Timer-based system information updates
- **Color-coded Metrics**: Different colors for CPU, GPU, RAM, etc.

### Fan Control
- **QCustomPlot Integration**: Professional fan curve graphing
- **Interactive Controls**: Sliders, combo boxes, and radio buttons
- **Profile Management**: Predefined and custom fan profiles
- **Table Widget**: Fan port configuration table

### RGB Lighting
- **Custom Sliders**: Styled sliders for speed and brightness control
- **Effect Selection**: Comprehensive lighting effect dropdown
- **Direction Controls**: Left/right direction toggle buttons
- **Visual Preview**: Product demo area for effect visualization

### SL Infinity Utility
- **FanWidget**: Custom animated fan visualization widgets
- **Grid Layout**: 4x4 fan configuration grid
- **Real-time Animation**: Color-changing fan animations
- **Port Management**: Individual port configuration

## Customization

### Styling
The application uses Qt stylesheets for theming. Key style classes:
- `#sidebar`: Left navigation sidebar
- `#monitoringCard`: System monitoring cards
- `#customSlider`: Custom slider controls
- `#fanWidget`: Fan visualization widgets

### Adding New Features
1. Create new page class in `src/pages/`
2. Add navigation button in `MainWindow::setupSidebar()`
3. Add page to content stack in `MainWindow::setupMainContent()`
4. Implement page-specific functionality

## Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- **Lian Li**: For the original L-Connect 3 design inspiration
- **Qt Framework**: For the excellent cross-platform GUI framework
- **QCustomPlot**: For the professional plotting capabilities

## Roadmap

- [ ] USB device detection and communication
- [ ] Hardware-specific fan control implementation
- [ ] RGB lighting hardware integration
- [ ] Configuration file persistence
- [ ] Plugin system for additional hardware support
- [ ] Multi-language support
- [ ] Advanced fan curve editing
- [ ] Hardware monitoring integration (lm-sensors, etc.)

## Support

For support, feature requests, or bug reports, please open an issue on GitHub.

---

**Note**: This is a UI recreation of L-Connect 3. Hardware communication features are not yet implemented and require additional development for actual device control.