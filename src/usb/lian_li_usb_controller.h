/*---------------------------------------------------------*\
|| lian_li_usb_controller.h                                |
||                                                         |
||   USB Controller for Lian Li UNI HUB devices           |
||   Based on OpenRGB implementation                       |
||                                                         |
||   This file is part of the L-Connect project           |
||   SPDX-License-Identifier: GPL-2.0-or-later            |
\*---------------------------------------------------------*/

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <libusb-1.0/libusb.h>

/*----------------------------------------------------------------------------*\
|| USB Device IDs (from OpenRGB)                                               |
\*----------------------------------------------------------------------------*/

#define LIAN_LI_VID                             0x0CF2
#define UNI_HUB_AL_PID                          0xA101  // AL v1
#define UNI_HUB_ALV2_PID                        0xA104  // AL v2
#define UNI_HUB_SLINF_PID                       0xA102  // SL Infinity
#define UNI_HUB_SLV2_PID                        0xA103  // SL v2
#define UNI_HUB_SLV2_V05_PID                    0xA105  // SL v2 v0.5

/*----------------------------------------------------------------------------*\
|| Channel and LED Configuration                                                |
\*----------------------------------------------------------------------------*/

enum
{
    UNIHUB_ALV2_CHANNEL_COUNT                   = 0x04,   // 4 channels
    UNIHUB_ALV2_CHANLED_COUNT                   = 0x50,   // 80 LEDs per channel
    UNIHUB_SLINF_CHANNEL_COUNT                  = 0x08,   // 8 channels
    UNIHUB_SLINF_CHANLED_COUNT                  = 0x60,   // 96 LEDs per channel
};

/*----------------------------------------------------------------------------*\
|| Global Action Addresses                                                      |
\*----------------------------------------------------------------------------*/

enum
{
    UNIHUB_ALV2_ACTION_ADDRESS                  = 0xE020, // Global action address
    UNIHUB_ALV2_COMMIT_ADDRESS                  = 0xE02F, // Global commit address
};

/*----------------------------------------------------------------------------*\
|| Channel-specific LED Addresses (ALv2)                                        |
\*----------------------------------------------------------------------------*/

enum
{
    // Channel 1
    UNIHUB_ALV2_LED_C1_ACTION_ADDRESS          = 0xE500,
    UNIHUB_ALV2_LED_C1_COMMIT_ADDRESS          = 0xE02F,
    UNIHUB_ALV2_LED_C1_MODE_ADDRESS            = 0xE021,
    UNIHUB_ALV2_LED_C1_SPEED_ADDRESS           = 0xE022,
    UNIHUB_ALV2_LED_C1_DIRECTION_ADDRESS       = 0xE023,
    UNIHUB_ALV2_LED_C1_BRIGHTNESS_ADDRESS      = 0xE029,

    // Channel 2
    UNIHUB_ALV2_LED_C2_ACTION_ADDRESS          = 0xE5F0,
    UNIHUB_ALV2_LED_C2_COMMIT_ADDRESS          = 0xE03F,
    UNIHUB_ALV2_LED_C2_MODE_ADDRESS            = 0xE031,
    UNIHUB_ALV2_LED_C2_SPEED_ADDRESS           = 0xE032,
    UNIHUB_ALV2_LED_C2_DIRECTION_ADDRESS       = 0xE033,
    UNIHUB_ALV2_LED_C2_BRIGHTNESS_ADDRESS      = 0xE039,

    // Channel 3
    UNIHUB_ALV2_LED_C3_ACTION_ADDRESS          = 0xE6E0,
    UNIHUB_ALV2_LED_C3_COMMIT_ADDRESS          = 0xE04F,
    UNIHUB_ALV2_LED_C3_MODE_ADDRESS            = 0xE041,
    UNIHUB_ALV2_LED_C3_SPEED_ADDRESS           = 0xE042,
    UNIHUB_ALV2_LED_C3_DIRECTION_ADDRESS       = 0xE043,
    UNIHUB_ALV2_LED_C3_BRIGHTNESS_ADDRESS      = 0xE049,

    // Channel 4
    UNIHUB_ALV2_LED_C4_ACTION_ADDRESS          = 0xE7D0,
    UNIHUB_ALV2_LED_C4_COMMIT_ADDRESS          = 0xE05F,
    UNIHUB_ALV2_LED_C4_MODE_ADDRESS            = 0xE051,
    UNIHUB_ALV2_LED_C4_SPEED_ADDRESS           = 0xE052,
    UNIHUB_ALV2_LED_C4_DIRECTION_ADDRESS       = 0xE053,
    UNIHUB_ALV2_LED_C4_BRIGHTNESS_ADDRESS      = 0xE059,
};

/*----------------------------------------------------------------------------*\
|| LED Modes (from OpenRGB)                                                     |
\*----------------------------------------------------------------------------*/

enum
{
    UNIHUB_LED_MODE_STATIC_COLOR               = 0x01,   // Static Color mode
    UNIHUB_LED_MODE_BREATHING                  = 0x02,   // Breathing mode
    UNIHUB_LED_MODE_RAINBOW_MORPH              = 0x04,   // Rainbow morph mode
    UNIHUB_LED_MODE_RAINBOW                    = 0x05,   // Rainbow mode
    UNIHUB_LED_MODE_STAGGERED                  = 0x18,   // Staggered mode
    UNIHUB_LED_MODE_TIDE                       = 0x1A,   // Tide mode
    UNIHUB_LED_MODE_RUNWAY                     = 0x1C,   // Runway mode
    UNIHUB_LED_MODE_MIXING                     = 0x1E,   // Mixing mode
    UNIHUB_LED_MODE_STACK                      = 0x20,   // Stack mode
    UNIHUB_LED_MODE_NEON                       = 0x22,   // Neon mode
    UNIHUB_LED_MODE_COLOR_CYCLE                = 0x23,   // Color Cycle mode
    UNIHUB_LED_MODE_METEOR                     = 0x24,   // Meteor mode
    UNIHUB_LED_MODE_VOICE                      = 0x26,   // Voice mode
    UNIHUB_LED_MODE_GROOVE                     = 0x27,   // Groove mode
    UNIHUB_LED_MODE_RENDER                     = 0x28,   // Render mode
    UNIHUB_LED_MODE_TUNNEL                     = 0x29,   // Tunnel mode
};

/*----------------------------------------------------------------------------*\
|| LED Speed Settings                                                           |
\*----------------------------------------------------------------------------*/

enum
{
    UNIHUB_LED_SPEED_000                       = 0x02,   // Very slow speed
    UNIHUB_LED_SPEED_025                       = 0x01,   // Rather slow speed
    UNIHUB_LED_SPEED_050                       = 0x00,   // Medium speed
    UNIHUB_LED_SPEED_075                       = 0xFF,   // Rather fast speed
    UNIHUB_LED_SPEED_100                       = 0xFE,   // Very fast speed
};

/*----------------------------------------------------------------------------*\
|| LED Direction Settings                                                       |
\*----------------------------------------------------------------------------*/

enum
{
    UNIHUB_LED_DIRECTION_LTR                   = 0x00,   // Left-to-Right direction
    UNIHUB_LED_DIRECTION_RTL                   = 0x01,   // Right-to-Left direction
};

/*----------------------------------------------------------------------------*\
|| LED Brightness Settings                                                      |
\*----------------------------------------------------------------------------*/

enum
{
    UNIHUB_LED_BRIGHTNESS_000                  = 0x08,   // Very dark (off)
    UNIHUB_LED_BRIGHTNESS_025                  = 0x03,   // Rather dark
    UNIHUB_LED_BRIGHTNESS_050                  = 0x02,   // Medium bright
    UNIHUB_LED_BRIGHTNESS_075                  = 0x01,   // Rather bright
    UNIHUB_LED_BRIGHTNESS_100                  = 0x00,   // Very bright
};

/*----------------------------------------------------------------------------*\
|| Color Structure (RBG format as required by Lian Li)                         |
\*----------------------------------------------------------------------------*/

struct LianLiColor
{
    uint8_t r;
    uint8_t b;  // Blue comes before Green!
    uint8_t g;
    
    LianLiColor() : r(0), b(0), g(0) {}
    LianLiColor(uint8_t red, uint8_t green, uint8_t blue) : r(red), b(blue), g(green) {}
    
    // Convert from standard RGB
    static LianLiColor fromRGB(uint8_t red, uint8_t green, uint8_t blue)
    {
        return LianLiColor(red, green, blue);
    }
};

/*----------------------------------------------------------------------------*\
|| Channel Configuration Structure                                               |
\*----------------------------------------------------------------------------*/

struct ChannelConfig
{
    uint8_t index;
    uint8_t fanCount;
    uint16_t ledActionAddress;
    uint16_t ledCommitAddress;
    uint16_t ledModeAddress;
    uint16_t ledSpeedAddress;
    uint16_t ledDirectionAddress;
    uint16_t ledBrightnessAddress;
    
    uint8_t ledMode;
    uint8_t ledSpeed;
    uint8_t ledDirection;
    uint8_t ledBrightness;
    
    std::vector<LianLiColor> colors;
    
    ChannelConfig() : index(0), fanCount(1), ledMode(UNIHUB_LED_MODE_STATIC_COLOR),
                     ledSpeed(UNIHUB_LED_SPEED_050), ledDirection(UNIHUB_LED_DIRECTION_LTR),
                     ledBrightness(UNIHUB_LED_BRIGHTNESS_100) {}
};

/*----------------------------------------------------------------------------*\
|| Main USB Controller Class                                                    |
\*----------------------------------------------------------------------------*/

class LianLiUSBController
{
public:
    enum DeviceType
    {
        UNI_HUB_ALV2,
        UNI_HUB_SL_INFINITY,
        UNI_HUB_AL,
        UNI_HUB_SLV2,
        UNKNOWN
    };

    LianLiUSBController();
    ~LianLiUSBController();

    // Device management
    bool Initialize();
    void Close();
    bool IsConnected() const;
    
    // Device information
    std::string GetDeviceName() const;
    std::string GetFirmwareVersion() const;
    std::string GetSerialNumber() const;
    DeviceType GetDeviceType() const;
    
    // Channel management
    bool SetChannelFanCount(uint8_t channel, uint8_t count);
    uint8_t GetChannelFanCount(uint8_t channel) const;
    
    // LED control
    bool SetChannelColors(uint8_t channel, const std::vector<LianLiColor>& colors);
    bool SetChannelMode(uint8_t channel, uint8_t mode);
    bool SetChannelSpeed(uint8_t channel, uint8_t speed);
    bool SetChannelDirection(uint8_t channel, uint8_t direction);
    bool SetChannelBrightness(uint8_t channel, uint8_t brightness);
    
    // Synchronization
    bool Synchronize();
    
    // Utility functions
    static LianLiColor RGBToRBG(uint8_t red, uint8_t green, uint8_t blue);
    static void RGBToRBG(uint8_t red, uint8_t green, uint8_t blue, uint8_t& r, uint8_t& b, uint8_t& g);

private:
    libusb_device_handle* m_handle;
    libusb_device* m_device;
    libusb_device_descriptor m_descriptor;
    
    DeviceType m_deviceType;
    std::string m_deviceName;
    std::string m_firmwareVersion;
    std::string m_serialNumber;
    std::string m_location;
    
    std::vector<ChannelConfig> m_channels;
    
    // Internal methods
    bool OpenDevice();
    void CloseDevice();
    bool SendConfig(uint16_t wIndex, const uint8_t* data, size_t length);
    bool SendCommit(uint16_t wIndex);
    std::string ReadVersion();
    std::string ReadSerial();
    void InitializeChannels();
    void SetupChannelAddresses();
    bool ValidateChannel(uint8_t channel) const;
    void ApplyColorLimiter(LianLiColor& color) const;
};
