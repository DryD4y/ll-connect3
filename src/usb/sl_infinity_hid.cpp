/*---------------------------------------------------------*\
|| sl_infinity_hid.cpp                                     |
||                                                         |
||   HID Controller for Lian Li SL Infinity (simplified)  |
||   Based on OpenRGB implementation                       |
||                                                         |
||   This file is part of the L-Connect project           |
||   SPDX-License-Identifier: GPL-2.0-or-later            |
\*---------------------------------------------------------*/

#include "sl_infinity_hid.h"
#include <iostream>
#include <cstring>
#include <fstream>
#include <chrono>
#include <thread>
#include <algorithm>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>

using namespace std::chrono_literals;

// HID Device Implementation
bool HIDDevice::Open(const std::string& devicePath) {
    Close();
    path = devicePath;
    
    fd = open(devicePath.c_str(), O_RDWR);
    if (fd < 0) {
        return false;
    }
    
    isOpen = true;
    return true;
}

void HIDDevice::Close() {
    if (fd >= 0) {
        close(fd);
        fd = -1;
    }
    isOpen = false;
}

bool HIDDevice::Write(const uint8_t* data, size_t length) {
    if (!isOpen || fd < 0) {
        return false;
    }
    
    ssize_t result = write(fd, data, length);
    return result == static_cast<ssize_t>(length);
}

// SL Infinity HID Controller Implementation
SLInfinityHIDController::SLInfinityHIDController() {
}

SLInfinityHIDController::~SLInfinityHIDController() {
    Close();
}

bool SLInfinityHIDController::Initialize() {
    if (!FindDevice()) {
        std::cerr << "SL Infinity device not found" << std::endl;
        return false;
    }
    
    m_deviceName = "Lian Li UNI HUB SL Infinity";
    m_firmwareVersion = "Unknown";
    m_serialNumber = "Unknown";
    
    return true;
}

void SLInfinityHIDController::Close() {
    m_device.Close();
}

bool SLInfinityHIDController::IsConnected() const {
    return m_device.IsOpen();
}

std::string SLInfinityHIDController::GetDeviceName() const {
    return m_deviceName;
}

std::string SLInfinityHIDController::GetFirmwareVersion() const {
    return m_firmwareVersion;
}

std::string SLInfinityHIDController::GetSerialNumber() const {
    return m_serialNumber;
}

static bool readSmallFile(const std::string& path, std::string& out) {
    std::ifstream f(path);
    if (!f.is_open()) return false;
    std::getline(f, out);
    // trim
    while (!out.empty() && (out.back()=='\n' || out.back()=='\r' || out.back()==' ')) out.pop_back();
    return true;
}

bool SLInfinityHIDController::FindDevice() {
    // Match by VID/PID via sysfs to select the correct hidraw node
    const std::string targetVid = "0cf2";
    const std::string targetPid = "a102";

    for (int i = 0; i < 32; i++) {
        std::string hidraw = "/dev/hidraw" + std::to_string(i);
        std::string sysBase = "/sys/class/hidraw/hidraw" + std::to_string(i) + "/device";

        // Walk up to find idVendor/idProduct
        std::string current = sysBase;
        for (int up = 0; up < 6; ++up) {
            std::string vidPath = current + "/idVendor";
            std::string pidPath = current + "/idProduct";
            std::string vid, pid;
            if (readSmallFile(vidPath, vid) && readSmallFile(pidPath, pid)) {
                // Lowercase for safety
                std::transform(vid.begin(), vid.end(), vid.begin(), ::tolower);
                std::transform(pid.begin(), pid.end(), pid.begin(), ::tolower);
                if (vid == targetVid && pid == targetPid) {
                    if (m_device.Open(hidraw)) {
                        return true;
                    }
                }
            }
            current += "/..";
        }
    }

    return false;
}

bool SLInfinityHIDController::SendStartAction(uint8_t channel, uint8_t numFans) {
    if (!m_device.IsOpen()) {
        return false;
    }

    uint8_t usb_buf[65];
    memset(usb_buf, 0x00, sizeof(usb_buf));

    usb_buf[0x00] = 0xE0;  // Transaction ID
    usb_buf[0x01] = 0x10;
    usb_buf[0x02] = 0x60;
    usb_buf[0x03] = 1 + (channel / 2); // Every fan-array uses two channels
    usb_buf[0x04] = 0x04; // Number of fans (hardcoded to 4 like OpenRGB)

    bool result = m_device.Write(usb_buf, sizeof(usb_buf));
    std::this_thread::sleep_for(5ms);
    return result;
}

bool SLInfinityHIDController::SendColorData(uint8_t channel, uint8_t numLeds, const uint8_t* ledData) {
    if (!m_device.IsOpen()) {
        return false;
    }

    uint8_t usb_buf[353];
    memset(usb_buf, 0x00, sizeof(usb_buf));

    usb_buf[0x00] = 0xE0;  // Transaction ID
    usb_buf[0x01] = 0x30 + channel; // Action + channel (30 = channel 1, 31 = channel 2, etc.)

    // Copy color data bytes (limit to buffer size)
    size_t dataSize = std::min(static_cast<size_t>(numLeds * 3), sizeof(usb_buf) - 2);
    memcpy(&usb_buf[0x02], ledData, dataSize);

    bool result = m_device.Write(usb_buf, sizeof(usb_buf));
    std::this_thread::sleep_for(5ms);
    return result;
}

bool SLInfinityHIDController::SendCommitAction(uint8_t channel, uint8_t effect, uint8_t speed, uint8_t direction, uint8_t brightness) {
    if (!m_device.IsOpen()) {
        return false;
    }

    uint8_t usb_buf[65];
    memset(usb_buf, 0x00, sizeof(usb_buf));

    usb_buf[0x00] = 0xE0;  // Transaction ID
    usb_buf[0x01] = 0x10 + channel; // Channel + device (10 = channel 1, 11 = channel 2, etc.)
    usb_buf[0x02] = effect;         // Effect
    usb_buf[0x03] = speed;          // Speed
    usb_buf[0x04] = direction;      // Direction
    usb_buf[0x05] = brightness;     // Brightness

    printf("SendCommitAction: channel=%d, effect=0x%02X, speed=0x%02X, direction=0x%02X, brightness=0x%02X\n", 
           channel, effect, speed, direction, brightness);

    bool result = m_device.Write(usb_buf, sizeof(usb_buf));
    std::this_thread::sleep_for(5ms);
    return result;
}

void SLInfinityHIDController::ApplyColorLimiter(SLInfinityColor& color) const {
    // Apply color limiter to protect LEDs (from OpenRGB)
    if ((color.r + color.b + color.g) > 460) {
        float scale = 460.0f / (color.r + color.b + color.g);
        color.r = static_cast<uint8_t>(color.r * scale);
        color.b = static_cast<uint8_t>(color.b * scale);
        color.g = static_cast<uint8_t>(color.g * scale);
    }
}

float SLInfinityHIDController::CalculateBrightnessLimit(const SLInfinityColor& color) const {
    if ((color.r + color.b + color.g) > 460) {
        return 460.0f / (color.r + color.b + color.g);
    }
    return 1.0f;
}

bool SLInfinityHIDController::SetChannelColors(uint8_t channel, const std::vector<SLInfinityColor>& colors) {
    printf("SetChannelColors: channel=%d, colors.size()=%zu\n", channel, colors.size());
    
    if (!m_device.IsOpen() || channel >= 8) {
        printf("SetChannelColors: Device not open or invalid channel\n");
        return false;
    }

    // Prepare LED data buffer (16 LEDs per fan, 4 fans = 64 LEDs total)
    uint8_t led_data[64 * 3]; // 64 LEDs * 3 bytes per LED
    memset(led_data, 0x00, sizeof(led_data));

    // For now, let's use a simple approach: fill all LEDs with the first color
    SLInfinityColor color = colors.empty() ? SLInfinityColor::fromRGB(0, 0, 0) : colors[0];
    ApplyColorLimiter(color);

    // Fill all 64 LEDs with the same color
    for (int i = 0; i < 64; i++) {
        int led_idx = i * 3;
        led_data[led_idx + 0] = color.r;  // Red
        led_data[led_idx + 1] = color.b;  // Blue (RBG format!)
        led_data[led_idx + 2] = color.g;  // Green
    }

    // Send start action (4 fans)
    printf("SetChannelColors: Sending start action for channel %d\n", channel);
    if (!SendStartAction(channel, 4)) {
        printf("SetChannelColors: SendStartAction failed for channel %d\n", channel);
        return false;
    }

    // Send color data (64 LEDs)
    printf("SetChannelColors: Sending color data for channel %d\n", channel);
    if (!SendColorData(channel, 64, led_data)) {
        printf("SetChannelColors: SendColorData failed for channel %d\n", channel);
        return false;
    }

    printf("SetChannelColors: Success for channel %d\n", channel);
    return true;
}

bool SLInfinityHIDController::SetChannelMode(uint8_t channel, uint8_t mode) {
    printf("SetChannelMode: channel=%d, mode=0x%02X\n", channel, mode);
    
    if (!m_device.IsOpen() || channel >= 8) {
        printf("SetChannelMode: Device not open or invalid channel\n");
        return false;
    }

    // Send commit action with the specified mode
    bool result = SendCommitAction(channel, mode, 0x00, 0x00, 0x00); // Static color mode
    printf("SetChannelMode: result=%s for channel %d\n", result ? "success" : "failed", channel);
    return result;
}

bool SLInfinityHIDController::TurnOffChannel(uint8_t channel) {
    if (!m_device.IsOpen() || channel >= 8) {
        return false;
    }

    // Set black color
    std::vector<SLInfinityColor> blackColor = {SLInfinityColor::fromRGB(0, 0, 0)};
    
    if (!SetChannelColors(channel, blackColor)) {
        return false;
    }

    // Send commit action to turn off (brightness 8 = 0% brightness)
    return SendCommitAction(channel, 0x01, 0x00, 0x00, 0x08); // Static color, off brightness
}

bool SLInfinityHIDController::TurnOffAllChannels() {
    bool success = true;
    for (uint8_t channel = 0; channel < 8; channel++) {
        success &= TurnOffChannel(channel);
    }
    return success;
}
