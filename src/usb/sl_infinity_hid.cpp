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
#include "../utils/debugutil.h"
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

    DEBUG_PRINTF("SendCommitAction: channel=%d, effect=0x%02X, speed=0x%02X, direction=0x%02X, brightness=0x%02X\n", 
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

bool SLInfinityHIDController::SetChannelColors(uint8_t channel, const std::vector<SLInfinityColor>& colors, float brightness, bool interleavedPattern) {
    DEBUG_PRINTF("SetChannelColors: channel=%d, colors.size()=%zu, brightness=%f, interleavedPattern=%d\n", channel, colors.size(), brightness, interleavedPattern);
    
    if (!m_device.IsOpen() || channel >= 8) {
        DEBUG_PRINTF("SetChannelColors: Device not open or invalid channel\n");
        return false;
    }

    // Prepare LED data buffer
    // OpenRGB uses (num_fans + 1) * 16 LEDs, so for 4 fans that's 80 LEDs
    // But we'll use 64 LEDs for 4 fans to match the hardware
    uint8_t led_data[80 * 3]; // 80 LEDs * 3 bytes per LED (max for 5 fans)
    memset(led_data, 0x00, sizeof(led_data));

    if (colors.empty()) {
        // No colors - fill with black
        memset(led_data, 0x00, sizeof(led_data));
    } else if (colors.size() == 1) {
        // Single color - fill all LEDs with this color
        // IMPORTANT: Don't apply color limiter first - calculate brightness scale using original color
        // then apply both brightness and limiter together (like OpenRGB does)
        SLInfinityColor color = colors[0];  // Keep original for brightness calculation
        
        // Calculate brightness scale like OpenRGB: brightness * infinityBrightnessLimit(color)
        // Use ORIGINAL color values (before limiting) for the brightness limit calculation
        float infinityBrightnessLimit = 1.0f;
        if ((color.r + color.b + color.g) > 460) {
            infinityBrightnessLimit = 460.0f / (color.r + color.b + color.g);
        }
        float color_brightness_scale = brightness * infinityBrightnessLimit;
        
        for (int i = 0; i < 64; i++) {
            int led_idx = i * 3;
            // Apply brightness and limiter together (like OpenRGB)
            led_data[led_idx + 0] = (unsigned char)(color.r * color_brightness_scale);  // Red
            led_data[led_idx + 1] = (unsigned char)(color.b * color_brightness_scale);  // Blue (RBG format!)
            led_data[led_idx + 2] = (unsigned char)(color.g * color_brightness_scale);  // Green
        }
        
        DEBUG_PRINTF("SetChannelColors: Set 1 color (single, solid) for channel %d: Color=%d,%d,%d brightness=%f (scale=%f, limit=%f)\n", 
                     channel, color.r, color.g, color.b, brightness, color_brightness_scale, infinityBrightnessLimit);
    } else if (colors.size() == 2) {
        // Two colors - for modes like Tide, Runway, Meteor
        // OpenRGB pattern: resize to 4 colors, fill unused with black, then interleave
        // This matches OpenRGB's SetChannelMode implementation exactly
        // IMPORTANT: Don't apply color limiter first - calculate brightness scale using original colors
        // then apply both brightness and limiter together (like OpenRGB does)
        SLInfinityColor color0 = colors[0];  // Keep original for brightness calculation
        SLInfinityColor color1 = colors[1];  // Keep original for brightness calculation
        SLInfinityColor color2 = SLInfinityColor::fromRGB(0, 0, 0);  // Black
        SLInfinityColor color3 = SLInfinityColor::fromRGB(0, 0, 0);  // Black
        
        // OpenRGB pattern: interleave colors across LEDs using formula: (i * 12) + (j * 3)
        // where i goes 0-5 (6 iterations) and j goes 0-3 (4 colors)
        // This creates: Color0 at 0,12,24,36,48,60 | Color1 at 3,15,27,39,51,63 | etc.
        // BUT: j=2 and j=3 are black (unused colors), so only j=0 and j=1 actually have data
        SLInfinityColor colorArray[4] = {color0, color1, color2, color3};
        
        // Fill the entire buffer with black first
        memset(led_data, 0x00, 80 * 3);
        
        for (unsigned int j = 0; j < 4; j++) {
            // Calculate brightness scale like OpenRGB: brightness * infinityBrightnessLimit(color)
            // Use ORIGINAL color values (before limiting) for the brightness limit calculation
            float infinityBrightnessLimit = 1.0f;
            if ((colorArray[j].r + colorArray[j].b + colorArray[j].g) > 460) {
                infinityBrightnessLimit = 460.0f / (colorArray[j].r + colorArray[j].b + colorArray[j].g);
            }
            float color_brightness_scale = brightness * infinityBrightnessLimit;
            
            for (unsigned int i = 0; i < 6; i++) {
                int cur_led_idx = (i * 12) + (j * 3);
                if (cur_led_idx < 80) {  // Ensure we don't exceed buffer
                    int led_data_idx = cur_led_idx * 3;
                    // Apply brightness and limiter together (like OpenRGB)
                    uint8_t r_val = (unsigned char)(colorArray[j].r * color_brightness_scale);
                    uint8_t b_val = (unsigned char)(colorArray[j].b * color_brightness_scale);
                    uint8_t g_val = (unsigned char)(colorArray[j].g * color_brightness_scale);
                    led_data[led_data_idx + 0] = r_val;  // Red
                    led_data[led_data_idx + 1] = b_val;  // Blue (RBG format!)
                    led_data[led_data_idx + 2] = g_val;  // Green
                    
                    // Debug: Log Color2 positions specifically (all 6 positions)
                    if (j == 1) {
                        DEBUG_PRINTF("  Color2 at LED[%d]: RGB=(%d,%d,%d) brightness_scale=%f (raw color: %d,%d,%d, limit=%f)\n", 
                                     cur_led_idx, r_val, g_val, b_val, color_brightness_scale,
                                     colorArray[j].r, colorArray[j].g, colorArray[j].b, infinityBrightnessLimit);
                    }
                }
            }
        }
        
        // EXPERIMENTAL: Meteor mode might need a denser pattern for Color2 to be visible
        // The interleaved pattern only has Color2 at 6 sparse positions (3,15,27,39,51,63)
        // Try filling adjacent positions with Color2 to create a more visible "meteor trail"
        // This might help the hardware firmware interpret the pattern correctly
        // Fill Color2 at positions 2,3,4 (adjacent to the main Color2 at 3)
        for (int offset = -1; offset <= 1; offset++) {
            for (unsigned int i = 0; i < 6; i++) {
                int base_led = (i * 12) + 3;  // Base Color2 position
                int cur_led_idx = base_led + offset;
                if (cur_led_idx >= 0 && cur_led_idx < 80) {
                    int led_data_idx = cur_led_idx * 3;
                    // Only overwrite if it's currently black (0,0,0)
                    if (led_data[led_data_idx] == 0 && led_data[led_data_idx+1] == 0 && led_data[led_data_idx+2] == 0) {
                        float infinityBrightnessLimit = 1.0f;
                        if ((color1.r + color1.b + color1.g) > 460) {
                            infinityBrightnessLimit = 460.0f / (color1.r + color1.b + color1.g);
                        }
                        float color_brightness_scale = brightness * infinityBrightnessLimit;
                        led_data[led_data_idx + 0] = (unsigned char)(color1.r * color_brightness_scale);
                        led_data[led_data_idx + 1] = (unsigned char)(color1.b * color_brightness_scale);
                        led_data[led_data_idx + 2] = (unsigned char)(color1.g * color_brightness_scale);
                    }
                }
            }
        }
        DEBUG_PRINTF("  Applied experimental Color2 density increase for Meteor mode\n");
        
        DEBUG_PRINTF("SetChannelColors: Set 2 colors for channel %d: Color1=%d,%d,%d Color2=%d,%d,%d brightness=%f (OpenRGB interleaved pattern)\n", 
                     channel,
                     color0.r, color0.g, color0.b,
                     color1.r, color1.g, color1.b,
                     brightness);
        
        // Debug: Dump a sample of the LED buffer to verify pattern
        DEBUG_PRINTF("  LED buffer sample (first 20 LEDs):\n");
        for (int i = 0; i < 20 && (i * 3) < (80 * 3); i++) {
            int idx = i * 3;
            DEBUG_PRINTF("    LED[%d]: R=%d G=%d B=%d\n", i, led_data[idx], led_data[idx+2], led_data[idx+1]);
        }
        DEBUG_PRINTF("  LED buffer sample (LEDs 60-69):\n");
        for (int i = 60; i < 70 && (i * 3) < (80 * 3); i++) {
            int idx = i * 3;
            DEBUG_PRINTF("    LED[%d]: R=%d G=%d B=%d\n", i, led_data[idx], led_data[idx+2], led_data[idx+1]);
        }
    } else if (colors.size() == 4) {
        // Four colors - Different patterns based on mode
        // interleavedPattern=true: For Tunnel (interleaved pattern like OpenRGB)
        // interleavedPattern=false: For Static (solid color per fan)
        
        // Ensure we have 4 colors
        std::vector<SLInfinityColor> colorArray = colors;
        while (colorArray.size() < 4) {
            colorArray.push_back(SLInfinityColor::fromRGB(0, 0, 0));
        }
        
        if (interleavedPattern) {
            // Interleaved pattern for Tunnel (matches OpenRGB)
            // Pattern: cur_led_idx = (i * 12) + (j * 3)
            // where i goes 0-5 (6 iterations) and j goes 0-3 (4 colors)
            // This creates: Color0 at 0,12,24,36,48,60 | Color1 at 3,15,27,39,51,63 | Color2 at 6,18,30,42,54,66 | Color3 at 9,21,33,45,57,69
            float brightness_value = brightness; // brightness is already 0-1.0
            
            for (unsigned int j = 0; j < 4; j++) {
                // Calculate brightness scale like OpenRGB: brightness * infinityBrightnessLimit(color)
                float infinityBrightnessLimit = 1.0f;
                if ((colorArray[j].r + colorArray[j].b + colorArray[j].g) > 460) {
                    infinityBrightnessLimit = 460.0f / (colorArray[j].r + colorArray[j].b + colorArray[j].g);
                }
                float color_brightness_scale = brightness_value * infinityBrightnessLimit;
                
                for (unsigned int i = 0; i < 6; i++) {
                    int cur_led_idx = (i * 12) + (j * 3);
                    if (cur_led_idx < 80) {
                        int led_data_idx = cur_led_idx * 3;
                        led_data[led_data_idx + 0] = (unsigned char)(colorArray[j].r * color_brightness_scale);
                        led_data[led_data_idx + 1] = (unsigned char)(colorArray[j].b * color_brightness_scale);
                        led_data[led_data_idx + 2] = (unsigned char)(colorArray[j].g * color_brightness_scale);
                    }
                }
            }
            
            DEBUG_PRINTF("SetChannelColors: Set 4 colors (interleaved pattern) for channel %d: Color1=%d,%d,%d Color2=%d,%d,%d Color3=%d,%d,%d Color4=%d,%d,%d brightness=%f\n", 
                         channel,
                         colorArray[0].r, colorArray[0].g, colorArray[0].b,
                         colorArray[1].r, colorArray[1].g, colorArray[1].b,
                         colorArray[2].r, colorArray[2].g, colorArray[2].b,
                         colorArray[3].r, colorArray[3].g, colorArray[3].b,
                         brightness);
        } else {
            // Solid per-fan pattern for Static mode
            // CRITICAL: Each fan needs ALL 16 LEDs set to the same color for solid appearance
            // Fan 1 = LEDs 0-15 gets colors[0] (repeated 16 times)
            // Fan 2 = LEDs 16-31 gets colors[1] (repeated 16 times)
            // Fan 3 = LEDs 32-47 gets colors[2] (repeated 16 times)
            // Fan 4 = LEDs 48-63 gets colors[3] (repeated 16 times)
            
            // Apply color limiter to all colors first
            for (int i = 0; i < 4; i++) {
                ApplyColorLimiter(colorArray[i]);
            }
            
            // For Static mode with 4 colors: Assign one solid color per fan (16 LEDs each)
            for (int fan = 0; fan < 4; fan++) {
                SLInfinityColor color = colorArray[fan];
                
                // Fill this fan's 16 LEDs with its assigned color (all LEDs same color)
                // Fan 0 = LEDs 0-15, Fan 1 = LEDs 16-31, Fan 2 = LEDs 32-47, Fan 3 = LEDs 48-63
                int base_led = fan * 16;
                for (int led = 0; led < 16; led++) {
                    int led_idx = (base_led + led) * 3;
                    led_data[led_idx + 0] = color.r;  // Red
                    led_data[led_idx + 1] = color.b;  // Blue (RBG format!)
                    led_data[led_idx + 2] = color.g;  // Green
                }
            }
            
            // Fill remaining LEDs with black (LEDs 64-79, in case we need 80 LEDs)
            for (int led = 64; led < 80; led++) {
                int led_idx = led * 3;
                led_data[led_idx + 0] = 0x00;  // Red
                led_data[led_idx + 1] = 0x00;  // Blue
                led_data[led_idx + 2] = 0x00;  // Green
            }
            
            DEBUG_PRINTF("SetChannelColors: Set 4 fan colors (Fan 1-4, solid per fan) for channel %d: Fan1=%d,%d,%d Fan2=%d,%d,%d Fan3=%d,%d,%d Fan4=%d,%d,%d\n", 
                         channel,
                         colorArray[0].r, colorArray[0].g, colorArray[0].b,
                         colorArray[1].r, colorArray[1].g, colorArray[1].b,
                         colorArray[2].r, colorArray[2].g, colorArray[2].b,
                         colorArray[3].r, colorArray[3].g, colorArray[3].b);
        }
    } else if (colors.size() == 3) {
        // Three colors - for ColorCycle mode
        // Distribute evenly: Each color gets ~21 LEDs
        SLInfinityColor color0 = colors[0];
        SLInfinityColor color1 = colors[1];
        SLInfinityColor color2 = colors[2];
        ApplyColorLimiter(color0);
        ApplyColorLimiter(color1);
        ApplyColorLimiter(color2);
        
        // Distribute colors: color0 (0-20), color1 (21-42), color2 (43-63)
        for (int i = 0; i < 64; i++) {
            SLInfinityColor color;
            if (i < 21) {
                color = color0;
            } else if (i < 43) {
                color = color1;
            } else {
                color = color2;
            }
            
            int led_idx = i * 3;
            led_data[led_idx + 0] = color.r;  // Red
            led_data[led_idx + 1] = color.b;  // Blue (RBG format!)
            led_data[led_idx + 2] = color.g;  // Green
        }
        
        DEBUG_PRINTF("SetChannelColors: Set 3 colors for channel %d: Color1=%d,%d,%d Color2=%d,%d,%d Color3=%d,%d,%d\n", 
                     channel,
                     color0.r, color0.g, color0.b,
                     color1.r, color1.g, color1.b,
                     color2.r, color2.g, color2.b);
    } else {
        // Multiple colors (5+ colors) - distribute them by cycling
        // For Static/Breathing with up to 6 colors, cycle through them
        for (int i = 0; i < 64; i++) {
            SLInfinityColor color = colors[i % colors.size()];
            ApplyColorLimiter(color);
            
            int led_idx = i * 3;
            led_data[led_idx + 0] = color.r;  // Red
            led_data[led_idx + 1] = color.b;  // Blue (RBG format!)
            led_data[led_idx + 2] = color.g;  // Green
        }
    }

    // Send start action - OpenRGB passes (num_fans + 1) but ignores it and hardcodes usb_buf[0x04] = 0x04
    // For 4 fans, OpenRGB calculates: fan_idx = (leds_count/16 - 1) = 3, then passes (fan_idx + 1) = 4
    // But SendStartAction ignores the parameter and hardcodes 4
    DEBUG_PRINTF("SetChannelColors: Sending start action for channel %d\n", channel);
    if (!SendStartAction(channel, 4)) {  // Pass 4 (OpenRGB ignores this and hardcodes it anyway)
        DEBUG_PRINTF("SetChannelColors: SendStartAction failed for channel %d\n", channel);
        return false;
    }

    // Send color data - OpenRGB sends (num_fans + 1) * 16 = 80 LEDs for 4 fans
    // This matches OpenRGB's SendColorData call exactly
    int num_leds_to_send = 80; // (num_fans + 1) * 16 = 80 for 4 fans (matches OpenRGB)
    DEBUG_PRINTF("SetChannelColors: Sending color data for channel %d (%d LEDs)\n", channel, num_leds_to_send);
    if (!SendColorData(channel, num_leds_to_send, led_data)) {
        DEBUG_PRINTF("SetChannelColors: SendColorData failed for channel %d\n", channel);
        return false;
    }

    DEBUG_PRINTF("SetChannelColors: Success for channel %d\n", channel);
    return true;
}

bool SLInfinityHIDController::SetChannelMode(uint8_t channel, uint8_t mode) {
    DEBUG_PRINTF("SetChannelMode: channel=%d, mode=0x%02X\n", channel, mode);
    
    if (!m_device.IsOpen() || channel >= 8) {
        DEBUG_PRINTF("SetChannelMode: Device not open or invalid channel\n");
        return false;
    }

    // Send commit action with the specified mode
    bool result = SendCommitAction(channel, mode, 0x00, 0x00, 0x00); // Static color mode
    DEBUG_PRINTF("SetChannelMode: result=%s for channel %d\n", result ? "success" : "failed", channel);
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
