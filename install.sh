#!/bin/bash

# L-Connect 3 Installation Script
# This script installs both the kernel driver and the Qt application

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
KERNEL_DIR="$SCRIPT_DIR/kernel"
BUILD_DIR="$SCRIPT_DIR/build"

# Function to print colored messages
print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if running as root (we'll use sudo when needed)
check_sudo() {
    if ! sudo -n true 2>/dev/null; then
        print_info "This script requires sudo privileges. You may be prompted for your password."
        sudo -v
    fi
}

# Detect Linux distribution
detect_distro() {
    if [ -f /etc/os-release ]; then
        . /etc/os-release
        DISTRO=$ID
        print_info "Detected distribution: $DISTRO"
    else
        print_error "Cannot detect Linux distribution. Assuming Debian-based."
        DISTRO="debian"
    fi
}

# Install dependencies based on distribution
install_dependencies() {
    print_info "Installing dependencies..."
    
    detect_distro
    
    if [[ "$DISTRO" == "ubuntu" || "$DISTRO" == "debian" ]]; then
        sudo apt update
        sudo apt install -y \
            build-essential \
            make \
            gcc \
            linux-headers-$(uname -r) \
            cmake \
            qt6-base-dev \
            qt6-charts-dev \
            lm-sensors \
            libusb-1.0-0-dev \
            libhidapi-dev \
            pkg-config
    elif [[ "$DISTRO" == "fedora" || "$DISTRO" == "rhel" || "$DISTRO" == "centos" ]]; then
        sudo dnf install -y \
            gcc \
            make \
            kernel-devel \
            kernel-headers \
            cmake \
            qt6-qtbase-devel \
            qt6-qtcharts-devel \
            lm_sensors \
            libusb-devel \
            hidapi-devel \
            pkgconfig
    elif [[ "$DISTRO" == "arch" || "$DISTRO" == "manjaro" ]]; then
        sudo pacman -S --needed \
            base-devel \
            linux-headers \
            cmake \
            qt6-base \
            qt6-charts \
            lm_sensors \
            libusb \
            hidapi \
            pkgconf
    else
        print_warning "Unknown distribution. Please install dependencies manually:"
        print_warning "- build-essential / base-devel"
        print_warning "- linux-headers / kernel-devel"
        print_warning "- cmake"
        print_warning "- qt6-base-dev / qt6-qtbase-devel"
        print_warning "- qt6-charts-dev / qt6-qtcharts-devel"
        print_warning "- lm-sensors / lm_sensors"
        print_warning "- libusb-1.0-0-dev / libusb-devel"
        print_warning "- libhidapi-dev / hidapi-devel"
        read -p "Press Enter to continue after installing dependencies manually..."
    fi
    
    print_success "Dependencies installed"
}

# Install kernel driver
install_kernel_driver() {
    print_info "Installing kernel driver..."
    
    if [ ! -d "$KERNEL_DIR" ]; then
        print_error "Kernel directory not found: $KERNEL_DIR"
        exit 1
    fi
    
    cd "$KERNEL_DIR"
    
    # Clean any previous build
    make clean 2>/dev/null || true
    
    # Build the module
    print_info "Building kernel module..."
    make
    
    if [ ! -f "Lian_Li_SL_INFINITY.ko" ]; then
        print_error "Kernel module build failed!"
        exit 1
    fi
    
    # Install using the Makefile (which handles depmod)
    print_info "Installing kernel module to system..."
    make install
    
    # Load the module
    print_info "Loading kernel module..."
    sudo rmmod Lian_Li_SL_INFINITY 2>/dev/null || true
    sudo modprobe Lian_Li_SL_INFINITY
    
    # Verify module is loaded
    if lsmod | grep -q "Lian_Li_SL_INFINITY"; then
        print_success "Kernel module loaded successfully"
    else
        print_error "Kernel module failed to load!"
        exit 1
    fi
    
    # Configure auto-load on boot
    print_info "Configuring kernel module to load on boot..."
    echo "Lian_Li_SL_INFINITY" | sudo tee /etc/modules-load.d/lian-li-sl-infinity.conf > /dev/null
    print_success "Kernel module will load automatically on boot"
    
    # Verify /proc entries exist
    if [ -d "/proc/Lian_li_SL_INFINITY" ]; then
        print_success "Kernel driver is working (found /proc/Lian_li_SL_INFINITY)"
    else
        print_warning "Warning: /proc/Lian_li_SL_INFINITY not found. Driver may not be working correctly."
    fi
    
    cd "$SCRIPT_DIR"
}

# Configure sensors
configure_sensors() {
    print_info "Configuring sensors..."
    
    if ! command -v sensors &> /dev/null; then
        print_warning "sensors command not found. Skipping sensor configuration."
        return
    fi
    
    if [ ! -f /etc/sensors3.conf ] && [ ! -f /etc/sensors.conf ]; then
        print_info "Running sensors-detect (this may be interactive)..."
        print_warning "You may be prompted for sensor detection. Accept defaults by pressing ENTER."
        if sudo sensors-detect --auto; then
            print_success "Sensors configured"
        else
            print_warning "Sensor detection cancelled or failed. You can run 'sudo sensors-detect' manually later."
        fi
    else
        print_info "Sensors already configured, skipping detection."
    fi
}

# Configure udev rule for HID access to SL-Infinity (RGB)
configure_udev() {
    print_info "Configuring udev rule for SL-Infinity HID access..."
    local RULE_FILE="/etc/udev/rules.d/60-lianli-sl-infinity.rules"
    echo 'SUBSYSTEM=="hidraw", ATTRS{idVendor}=="0cf2", ATTRS{idProduct}=="a102", TAG+="uaccess", MODE="0666"' | sudo tee "$RULE_FILE" > /dev/null
    sudo udevadm control --reload
    sudo udevadm trigger
    print_success "udev rule installed at $RULE_FILE"
}

# Install Qt application
install_application() {
    print_info "Installing L-Connect3 application..."
    
    # Create build directory
    if [ -d "$BUILD_DIR" ]; then
        print_info "Cleaning previous build..."
        rm -rf "$BUILD_DIR"
    fi
    
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    
    # Configure with CMake
    print_info "Configuring build with CMake..."
    cmake -DCMAKE_INSTALL_PREFIX=/usr ..
    
    # Build
    print_info "Building application..."
    make -j$(nproc)
    
    # Install
    print_info "Installing application to system..."
    sudo make install
    
    # Verify installation
    if command -v LLConnect3 &> /dev/null; then
        print_success "Application installed successfully"
    else
        print_error "Application installation verification failed!"
        exit 1
    fi
    
    cd "$SCRIPT_DIR"
}

# Verify installation
verify_installation() {
    print_info "Verifying installation..."
    
    # Check kernel module
    if lsmod | grep -q "Lian_Li_SL_INFINITY"; then
        print_success "✓ Kernel module is loaded"
    else
        print_error "✗ Kernel module is not loaded"
    fi
    
    # Check /proc entries
    if [ -d "/proc/Lian_li_SL_INFINITY" ]; then
        print_success "✓ Kernel driver /proc entries exist"
    else
        print_error "✗ Kernel driver /proc entries not found"
    fi
    
    # Check auto-load configuration
    if [ -f "/etc/modules-load.d/lian-li-sl-infinity.conf" ]; then
        print_success "✓ Auto-load configuration exists"
    else
        print_warning "✗ Auto-load configuration not found"
    fi
    
    # Check application
    if command -v LLConnect3 &> /dev/null; then
        print_success "✓ LLConnect3 application is installed"
    else
        print_error "✗ LLConnect3 application not found"
    fi
    
    # Check desktop file
    if [ -f "/usr/share/applications/lconnect3.desktop" ]; then
        print_success "✓ Desktop file installed"
    else
        print_warning "✗ Desktop file not found"
    fi
}

# Main installation process
main() {
    echo ""
    echo "=========================================="
    echo "  L-Connect 3 Installation Script"
    echo "=========================================="
    echo ""
    
    check_sudo
    
    print_info "Starting installation process..."
    echo ""
    
    # Step 1: Install dependencies
    install_dependencies
    echo ""
    
    # Step 2: Install kernel driver
    install_kernel_driver
    echo ""
    
    # Step 3: Configure sensors
    configure_sensors
    echo ""
    
    # Step 4: Configure udev (RGB HID access)
    configure_udev
    echo ""
    
    # Step 5: Install application
    install_application
    echo ""
    
    # Step 6: Verify installation
    verify_installation
    echo ""
    
    print_success "Installation complete!"
    echo ""
    print_info "You can now:"
    echo "  - Launch the application: LLConnect3"
    echo "  - Control fans via /proc/Lian_li_SL_INFINITY/Port_X/fan_speed"
    echo "  - Check module status: lsmod | grep Lian_Li"
    echo ""
    print_warning "Note: After kernel updates, you may need to rebuild the kernel module:"
    echo "  cd $KERNEL_DIR && make clean && make && sudo make install"
    echo ""
}

# Run main function
main

