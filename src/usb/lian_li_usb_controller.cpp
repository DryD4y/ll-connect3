/*---------------------------------------------------------*\
|| lian_li_usb_controller.cpp                              |
||                                                         |
||   USB Controller for Lian Li UNI HUB devices           |
||   Based on OpenRGB implementation                       |
||                                                         |
||   This file is part of the L-Connect project           |
||   SPDX-License-Identifier: GPL-2.0-or-later            |
\*---------------------------------------------------------*/

#include "lian_li_usb_controller.h"
#include <cstring>
#include <chrono>
#include <thread>
#include <iostream>

using namespace std::chrono_literals;

LianLiUSBController::LianLiUSBController()
    : m_handle(nullptr)
    , m_device(nullptr)
    , m_deviceType(UNKNOWN)
{
    memset(&m_descriptor, 0, sizeof(m_descriptor));
}

LianLiUSBController::~LianLiUSBController()
{
    Close();
}

bool LianLiUSBController::Initialize()
{
    // Initialize libusb
    int ret = libusb_init(nullptr);
    if (ret < 0)
    {
        std::cerr << "Failed to initialize libusb: " << libusb_error_name(ret) << std::endl;
        return false;
    }

    // Open device
    if (!OpenDevice())
    {
        return false;
    }

    // Initialize channels
    InitializeChannels();

    return true;
}

void LianLiUSBController::Close()
{
    CloseDevice();
    libusb_exit(nullptr);
}

bool LianLiUSBController::IsConnected() const
{
    return m_handle != nullptr;
}

std::string LianLiUSBController::GetDeviceName() const
{
    return m_deviceName;
}

std::string LianLiUSBController::GetFirmwareVersion() const
{
    return m_firmwareVersion;
}

std::string LianLiUSBController::GetSerialNumber() const
{
    return m_serialNumber;
}

LianLiUSBController::DeviceType LianLiUSBController::GetDeviceType() const
{
    return m_deviceType;
}

bool LianLiUSBController::OpenDevice()
{
    libusb_device** devices = nullptr;
    ssize_t ret = libusb_get_device_list(nullptr, &devices);
    
    if (ret < 0)
    {
        std::cerr << "Failed to get device list: " << libusb_error_name(ret) << std::endl;
        return false;
    }

    ssize_t deviceCount = ret;
    bool found = false;

    for (int i = 0; i < deviceCount; i++)
    {
        libusb_device* device = devices[i];
        libusb_device_descriptor descriptor;
        
        ret = libusb_get_device_descriptor(device, &descriptor);
        if (ret < 0)
        {
            continue;
        }

        // Check if this is a Lian Li device
        if (descriptor.idVendor == LIAN_LI_VID)
        {
            switch (descriptor.idProduct)
            {
                case UNI_HUB_ALV2_PID:
                    m_deviceType = UNI_HUB_ALV2;
                    m_deviceName = "Lian Li UNI HUB ALv2";
                    found = true;
                    break;
                case UNI_HUB_SLINF_PID:
                    m_deviceType = UNI_HUB_SL_INFINITY;
                    m_deviceName = "Lian Li UNI HUB SL Infinity";
                    found = true;
                    break;
                case UNI_HUB_AL_PID:
                    m_deviceType = UNI_HUB_AL;
                    m_deviceName = "Lian Li UNI HUB AL";
                    found = true;
                    break;
                case UNI_HUB_SLV2_PID:
                case UNI_HUB_SLV2_V05_PID:
                    m_deviceType = UNI_HUB_SLV2;
                    m_deviceName = "Lian Li UNI HUB SLv2";
                    found = true;
                    break;
                default:
                    continue;
            }

            if (found)
            {
                m_device = device;
                m_descriptor = descriptor;
                
                // Open the device
                ret = libusb_open(device, &m_handle);
                if (ret < 0)
                {
                    std::cerr << "Failed to open device: " << libusb_error_name(ret) << std::endl;
                    found = false;
                    continue;
                }

                // Read device information
                m_firmwareVersion = ReadVersion();
                m_serialNumber = ReadSerial();
                
                // Get USB port information
                uint8_t ports[7];
                ret = libusb_get_port_numbers(device, ports, sizeof(ports));
                if (ret > 0)
                {
                    m_location = "USB: ";
                    for (int j = 0; j < ret; j++)
                    {
                        m_location += std::to_string(ports[j]);
                        m_location.push_back(':');
                    }
                    m_location.pop_back();
                }

                break;
            }
        }
    }

    libusb_free_device_list(devices, 1);
    return found;
}

void LianLiUSBController::CloseDevice()
{
    if (m_handle != nullptr)
    {
        libusb_close(m_handle);
        m_handle = nullptr;
    }
    m_device = nullptr;
}

bool LianLiUSBController::SendConfig(uint16_t wIndex, const uint8_t* data, size_t length)
{
    if (m_handle == nullptr)
    {
        return false;
    }

    size_t ret = libusb_control_transfer(
        m_handle,           // dev_handle
        0x40,               // bmRequestType (Host to Device, Vendor, Device)
        0x80,               // bRequest (Custom vendor request)
        0x00,               // wValue
        wIndex,             // wIndex
        const_cast<uint8_t*>(data),  // data
        static_cast<uint16_t>(length), // wLength
        1000                // timeout
    );

    // Add small delay as used in OpenRGB
    std::this_thread::sleep_for(5ms);

    return ret == length;
}

bool LianLiUSBController::SendCommit(uint16_t wIndex)
{
    uint8_t config[1] = { 0x01 };
    bool result = SendConfig(wIndex, config, sizeof(config));
    std::this_thread::sleep_for(5ms);
    return result;
}

std::string LianLiUSBController::ReadVersion()
{
    if (m_handle == nullptr)
    {
        return "";
    }

    uint8_t buffer[5];
    size_t ret = libusb_control_transfer(
        m_handle,           // dev_handle
        0xC0,               // bmRequestType (Device to Host, Vendor, Device)
        0x81,               // bRequest
        0x00,               // wValue
        0xB500,             // wIndex
        buffer,             // data
        sizeof(buffer),     // wLength
        1000                // timeout
    );

    if (ret != sizeof(buffer))
    {
        return "";
    }

    char version[15];
    int vlength = std::snprintf(version, sizeof(version), "%x.%x.%x.%x.%x", 
                                buffer[0], buffer[1], buffer[2], buffer[3], buffer[4]);
    
    return std::string(version, vlength);
}

std::string LianLiUSBController::ReadSerial()
{
    if (m_handle == nullptr)
    {
        return "";
    }

    char serialStr[64];
    int ret = libusb_get_string_descriptor_ascii(
        m_handle, 
        m_descriptor.iSerialNumber, 
        reinterpret_cast<unsigned char*>(serialStr), 
        sizeof(serialStr)
    );

    if (ret > 0)
    {
        return std::string(serialStr, ret);
    }

    return "";
}

void LianLiUSBController::InitializeChannels()
{
    m_channels.clear();
    
    uint8_t channelCount = (m_deviceType == UNI_HUB_SL_INFINITY) ? 
                          UNIHUB_SLINF_CHANNEL_COUNT : UNIHUB_ALV2_CHANNEL_COUNT;
    
    // Limit channel count to prevent issues
    channelCount = std::min(channelCount, static_cast<uint8_t>(8));
    
    for (uint8_t i = 0; i < channelCount; i++)
    {
        ChannelConfig channel;
        channel.index = i;
        channel.fanCount = 1;  // Start with 1 fan per channel
        
        // Set default values
        channel.ledMode = UNIHUB_LED_MODE_STATIC_COLOR;
        channel.ledSpeed = UNIHUB_LED_SPEED_050;
        channel.ledDirection = UNIHUB_LED_DIRECTION_LTR;
        channel.ledBrightness = UNIHUB_LED_BRIGHTNESS_100;
        
        // Initialize colors (all black) - limit to reasonable size
        uint8_t maxLeds = (m_deviceType == UNI_HUB_SL_INFINITY) ? 
                         UNIHUB_SLINF_CHANLED_COUNT : UNIHUB_ALV2_CHANLED_COUNT;
        maxLeds = std::min(maxLeds, static_cast<uint8_t>(96));  // Reasonable limit
        channel.colors.resize(maxLeds, LianLiColor(0, 0, 0));
        
        m_channels.push_back(channel);
    }
    
    SetupChannelAddresses();
}

void LianLiUSBController::SetupChannelAddresses()
{
    if (m_deviceType == UNI_HUB_ALV2 || m_deviceType == UNI_HUB_AL)
    {
        // ALv2/AL channel addresses
        if (m_channels.size() >= 1)
        {
            m_channels[0].ledActionAddress = UNIHUB_ALV2_LED_C1_ACTION_ADDRESS;
            m_channels[0].ledCommitAddress = UNIHUB_ALV2_LED_C1_COMMIT_ADDRESS;
            m_channels[0].ledModeAddress = UNIHUB_ALV2_LED_C1_MODE_ADDRESS;
            m_channels[0].ledSpeedAddress = UNIHUB_ALV2_LED_C1_SPEED_ADDRESS;
            m_channels[0].ledDirectionAddress = UNIHUB_ALV2_LED_C1_DIRECTION_ADDRESS;
            m_channels[0].ledBrightnessAddress = UNIHUB_ALV2_LED_C1_BRIGHTNESS_ADDRESS;
        }
        
        if (m_channels.size() >= 2)
        {
            m_channels[1].ledActionAddress = UNIHUB_ALV2_LED_C2_ACTION_ADDRESS;
            m_channels[1].ledCommitAddress = UNIHUB_ALV2_LED_C2_COMMIT_ADDRESS;
            m_channels[1].ledModeAddress = UNIHUB_ALV2_LED_C2_MODE_ADDRESS;
            m_channels[1].ledSpeedAddress = UNIHUB_ALV2_LED_C2_SPEED_ADDRESS;
            m_channels[1].ledDirectionAddress = UNIHUB_ALV2_LED_C2_DIRECTION_ADDRESS;
            m_channels[1].ledBrightnessAddress = UNIHUB_ALV2_LED_C2_BRIGHTNESS_ADDRESS;
        }
        
        if (m_channels.size() >= 3)
        {
            m_channels[2].ledActionAddress = UNIHUB_ALV2_LED_C3_ACTION_ADDRESS;
            m_channels[2].ledCommitAddress = UNIHUB_ALV2_LED_C3_COMMIT_ADDRESS;
            m_channels[2].ledModeAddress = UNIHUB_ALV2_LED_C3_MODE_ADDRESS;
            m_channels[2].ledSpeedAddress = UNIHUB_ALV2_LED_C3_SPEED_ADDRESS;
            m_channels[2].ledDirectionAddress = UNIHUB_ALV2_LED_C3_DIRECTION_ADDRESS;
            m_channels[2].ledBrightnessAddress = UNIHUB_ALV2_LED_C3_BRIGHTNESS_ADDRESS;
        }
        
        if (m_channels.size() >= 4)
        {
            m_channels[3].ledActionAddress = UNIHUB_ALV2_LED_C4_ACTION_ADDRESS;
            m_channels[3].ledCommitAddress = UNIHUB_ALV2_LED_C4_COMMIT_ADDRESS;
            m_channels[3].ledModeAddress = UNIHUB_ALV2_LED_C4_MODE_ADDRESS;
            m_channels[3].ledSpeedAddress = UNIHUB_ALV2_LED_C4_SPEED_ADDRESS;
            m_channels[3].ledDirectionAddress = UNIHUB_ALV2_LED_C4_DIRECTION_ADDRESS;
            m_channels[3].ledBrightnessAddress = UNIHUB_ALV2_LED_C4_BRIGHTNESS_ADDRESS;
        }
    }
    // TODO: Add SL Infinity channel addresses if needed
}

bool LianLiUSBController::ValidateChannel(uint8_t channel) const
{
    return channel < m_channels.size();
}

void LianLiUSBController::ApplyColorLimiter(LianLiColor& color) const
{
    // Apply color limiter to protect LEDs (from OpenRGB)
    if ((color.r > 153) && (color.r == color.b) && (color.r == color.g))
    {
        color.r = 153;
        color.b = 153;
        color.g = 153;
    }
}

bool LianLiUSBController::SetChannelFanCount(uint8_t channel, uint8_t count)
{
    if (!ValidateChannel(channel))
    {
        return false;
    }

    m_channels[channel].fanCount = count;
    return true;
}

uint8_t LianLiUSBController::GetChannelFanCount(uint8_t channel) const
{
    if (!ValidateChannel(channel))
    {
        return 0;
    }

    return m_channels[channel].fanCount;
}

bool LianLiUSBController::SetChannelColors(uint8_t channel, const std::vector<LianLiColor>& colors)
{
    if (!ValidateChannel(channel))
    {
        return false;
    }

    ChannelConfig& ch = m_channels[channel];
    size_t maxLeds = ch.colors.size();
    size_t count = std::min(colors.size(), maxLeds);

    // Copy colors with RBG format
    for (size_t i = 0; i < count; i++)
    {
        ch.colors[i] = colors[i];
        ApplyColorLimiter(ch.colors[i]);
    }

    // Set remaining LEDs to black
    for (size_t i = count; i < maxLeds; i++)
    {
        ch.colors[i] = LianLiColor(0, 0, 0);
    }

    return true;
}

bool LianLiUSBController::SetChannelMode(uint8_t channel, uint8_t mode)
{
    if (!ValidateChannel(channel))
    {
        return false;
    }

    m_channels[channel].ledMode = mode;
    return true;
}

bool LianLiUSBController::SetChannelSpeed(uint8_t channel, uint8_t speed)
{
    if (!ValidateChannel(channel))
    {
        return false;
    }

    m_channels[channel].ledSpeed = speed;
    return true;
}

bool LianLiUSBController::SetChannelDirection(uint8_t channel, uint8_t direction)
{
    if (!ValidateChannel(channel))
    {
        return false;
    }

    m_channels[channel].ledDirection = direction;
    return true;
}

bool LianLiUSBController::SetChannelBrightness(uint8_t channel, uint8_t brightness)
{
    if (!ValidateChannel(channel))
    {
        return false;
    }

    m_channels[channel].ledBrightness = brightness;
    return true;
}

bool LianLiUSBController::Synchronize()
{
    if (m_handle == nullptr)
    {
        return false;
    }

    // Send initialization command (from OpenRGB)
    uint8_t config_initialization[16];
    memset(config_initialization, 0x00, sizeof(config_initialization));
    config_initialization[0x0F] = 0x43;  // Control data
    config_initialization[0x0F] = 0x01;  // Ending data

    if (!SendConfig(UNIHUB_ALV2_ACTION_ADDRESS, config_initialization, sizeof(config_initialization)))
    {
        return false;
    }

    // Configure fan counts for each channel
    for (const ChannelConfig& channel : m_channels)
    {
        uint8_t anyFanCount = channel.fanCount;
        if (anyFanCount == 0)
        {
            anyFanCount = 1;  // Uni Hub doesn't know zero fans
        }

        uint8_t config_fan_count[16];
        memset(config_fan_count, 0x00, sizeof(config_fan_count));
        config_fan_count[0x01] = 0x40;                    // Control data
        config_fan_count[0x02] = channel.index + 1;       // Channel
        config_fan_count[0x03] = anyFanCount + 1;         // Number of fans
        config_fan_count[0x0F] = 0x01;                    // Ending data

        if (!SendConfig(UNIHUB_ALV2_ACTION_ADDRESS, config_fan_count, sizeof(config_fan_count)))
        {
            return false;
        }
    }

    // Configure LED settings for each channel
    for (const ChannelConfig& channel : m_channels)
    {
        if (channel.fanCount > 0)
        {
            // Send color data for each fan (20 LEDs per fan)
            // Limit fan count to prevent buffer overflows
            uint8_t maxFans = std::min(channel.fanCount, static_cast<uint8_t>(4));
            
            for (uint8_t fan_idx = 0; fan_idx < maxFans; fan_idx++)
            {
                uint8_t fan_config_colors[60];  // Fixed: 20 LEDs * 3 bytes = 60 bytes
                size_t start_idx = fan_idx * 20;
                
                // Initialize buffer to zero
                memset(fan_config_colors, 0, sizeof(fan_config_colors));
                
                // Copy 20 LEDs worth of color data (60 bytes: 20 * 3 bytes per LED)
                // Add bounds checking to prevent overflow
                for (size_t i = 0; i < 20 && (start_idx + i) < channel.colors.size() && (i * 3 + 2) < sizeof(fan_config_colors); i++)
                {
                    fan_config_colors[i * 3] = channel.colors[start_idx + i].r;
                    fan_config_colors[i * 3 + 1] = channel.colors[start_idx + i].b;
                    fan_config_colors[i * 3 + 2] = channel.colors[start_idx + i].g;
                }

                if (!SendConfig(channel.ledActionAddress + (60 * fan_idx), fan_config_colors, sizeof(fan_config_colors)))
                {
                    return false;
                }
            }

            // Send LED mode configuration
            uint8_t led_mode_config[16];
            memset(led_mode_config, 0x00, sizeof(led_mode_config));
            led_mode_config[0x01] = 0x40;                  // Control data
            led_mode_config[0x02] = channel.index + 1;     // Channel
            led_mode_config[0x03] = channel.ledMode;       // LED mode
            led_mode_config[0x0F] = 0x01;                  // Ending data

            if (!SendConfig(channel.ledModeAddress, led_mode_config, sizeof(led_mode_config)))
            {
                return false;
            }

            // Send LED speed configuration
            uint8_t led_speed_config[16];
            memset(led_speed_config, 0x00, sizeof(led_speed_config));
            led_speed_config[0x01] = 0x40;                 // Control data
            led_speed_config[0x02] = channel.index + 1;    // Channel
            led_speed_config[0x03] = channel.ledSpeed;     // LED speed
            led_speed_config[0x0F] = 0x01;                 // Ending data

            if (!SendConfig(channel.ledSpeedAddress, led_speed_config, sizeof(led_speed_config)))
            {
                return false;
            }

            // Send LED direction configuration
            uint8_t led_direction_config[16];
            memset(led_direction_config, 0x00, sizeof(led_direction_config));
            led_direction_config[0x01] = 0x40;             // Control data
            led_direction_config[0x02] = channel.index + 1; // Channel
            led_direction_config[0x03] = channel.ledDirection; // LED direction
            led_direction_config[0x0F] = 0x01;             // Ending data

            if (!SendConfig(channel.ledDirectionAddress, led_direction_config, sizeof(led_direction_config)))
            {
                return false;
            }

            // Send LED brightness configuration
            uint8_t led_brightness_config[16];
            memset(led_brightness_config, 0x00, sizeof(led_brightness_config));
            led_brightness_config[0x01] = 0x40;            // Control data
            led_brightness_config[0x02] = channel.index + 1; // Channel
            led_brightness_config[0x03] = channel.ledBrightness; // LED brightness
            led_brightness_config[0x0F] = 0x01;            // Ending data

            if (!SendConfig(channel.ledBrightnessAddress, led_brightness_config, sizeof(led_brightness_config)))
            {
                return false;
            }

            // Commit the configuration
            if (!SendCommit(channel.ledCommitAddress))
            {
                return false;
            }
        }
    }

    return true;
}

LianLiColor LianLiUSBController::RGBToRBG(uint8_t red, uint8_t green, uint8_t blue)
{
    return LianLiColor(red, green, blue);
}

void LianLiUSBController::RGBToRBG(uint8_t red, uint8_t green, uint8_t blue, uint8_t& r, uint8_t& b, uint8_t& g)
{
    r = red;
    b = blue;  // Blue comes before Green in RBG format
    g = green;
}
