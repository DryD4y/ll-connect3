/*---------------------------------------------------------*\
|| sl_infinity_hid.h                                       |
||                                                         |
||   HID Controller for Lian Li SL Infinity (simplified)  |
||   Based on OpenRGB implementation                       |
||                                                         |
||   This file is part of the L-Connect project           |
||   SPDX-License-Identifier: GPL-2.0-or-later            |
\*---------------------------------------------------------*/

#pragma once

#include <cstdint>
#include <string>
#include <vector>

// Simplified HID interface without external dependencies
struct HIDDevice {
    int fd;
    std::string path;
    bool isOpen;
    
    HIDDevice() : fd(-1), isOpen(false) {}
    ~HIDDevice() { Close(); }
    
    bool Open(const std::string& devicePath);
    void Close();
    bool Write(const uint8_t* data, size_t length);
    bool IsOpen() const { return isOpen; }
};

// SL Infinity Color Structure (RBG format)
struct SLInfinityColor {
    uint8_t r;
    uint8_t b;  // Blue comes before Green!
    uint8_t g;
    
    SLInfinityColor() : r(0), b(0), g(0) {}
    SLInfinityColor(uint8_t red, uint8_t green, uint8_t blue) : r(red), b(blue), g(green) {}
    
    static SLInfinityColor fromRGB(uint8_t red, uint8_t green, uint8_t blue) {
        return SLInfinityColor(red, green, blue);
    }
};

// SL Infinity HID Controller
class SLInfinityHIDController {
public:
    SLInfinityHIDController();
    ~SLInfinityHIDController();

    // Device management
    bool Initialize();
    void Close();
    bool IsConnected() const;
    
    // Device information
    std::string GetDeviceName() const;
    std::string GetFirmwareVersion() const;
    std::string GetSerialNumber() const;
    
    // LED control
    // patternType: true = interleaved (for Tunnel), false = solid per fan (for Static)
    bool SetChannelColors(uint8_t channel, const std::vector<SLInfinityColor>& colors, float brightness = 1.0f, bool interleavedPattern = false);
    bool SetChannelMode(uint8_t channel, uint8_t mode);
    bool TurnOffChannel(uint8_t channel);
    bool TurnOffAllChannels();
    
    // Public methods for testing
    bool SendCommitAction(uint8_t channel, uint8_t effect, uint8_t speed, uint8_t direction, uint8_t brightness);

private:
    HIDDevice m_device;
    std::string m_deviceName;
    std::string m_firmwareVersion;
    std::string m_serialNumber;
    
    // Internal methods
    bool FindDevice();
    bool SendStartAction(uint8_t channel, uint8_t numFans);
    bool SendColorData(uint8_t channel, uint8_t numLeds, const uint8_t* ledData);
    void ApplyColorLimiter(SLInfinityColor& color) const;
    float CalculateBrightnessLimit(const SLInfinityColor& color) const;
};
