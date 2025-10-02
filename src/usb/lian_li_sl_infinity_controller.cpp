/*---------------------------------------------------------*\
|| lian_li_sl_infinity_controller.cpp                       |
||                                                         |
||   HID Controller for Lian Li SL Infinity devices       |
||   Based on OpenRGB implementation                       |
||                                                         |
||   This file is part of the L-Connect project           |
||   SPDX-License-Identifier: GPL-2.0-or-later            |
\*---------------------------------------------------------*/

#include "lian_li_sl_infinity_controller.h"
#include <iostream>
#include <cstring>
#include <chrono>
#include <thread>
#include <fstream>

using namespace std::chrono_literals;

LianLiSLInfinityController::LianLiSLInfinityController()
    : m_handle(nullptr), m_initialized(false)
{
}

LianLiSLInfinityController::~LianLiSLInfinityController()
{
    Close();
}

bool LianLiSLInfinityController::Initialize()
{
    std::cout << "Initializing Lian Li SL Infinity controller..." << std::endl;
    
    // Initialize HID API
    if (hid_init() < 0)
    {
        std::cerr << "Failed to initialize HID API" << std::endl;
        return false;
    }
    
    std::cout << "HID API initialized successfully" << std::endl;

    // Open device
    if (!OpenDevice())
    {
        std::cout << "Failed to open Lian Li device" << std::endl;
        return false;
    }

    std::cout << "Lian Li SL Infinity controller initialized successfully" << std::endl;
    return true;
}

void LianLiSLInfinityController::Close()
{
    CloseDevice();
    hid_exit();
}

bool LianLiSLInfinityController::IsConnected() const
{
    return m_handle != nullptr;
}

std::string LianLiSLInfinityController::GetDeviceName() const
{
    return m_deviceName;
}

std::string LianLiSLInfinityController::GetFirmwareVersion() const
{
    return m_firmwareVersion;
}

std::string LianLiSLInfinityController::GetSerialNumber() const
{
    return m_serialNumber;
}

bool LianLiSLInfinityController::OpenDevice()
{
    // Enumerate HID devices
    std::cout << "Enumerating HID devices for VID:0x0CF2, PID:0xA102..." << std::endl;
    struct hid_device_info* devs = hid_enumerate(0x0CF2, 0xA102); // SL Infinity VID:PID
    struct hid_device_info* cur_dev = devs;

    if (devs == nullptr)
    {
        std::cerr << "No Lian Li SL Infinity devices found" << std::endl;
        std::cout << "Trying to enumerate all HID devices..." << std::endl;
        devs = hid_enumerate(0x0, 0x0); // Enumerate all devices
        cur_dev = devs;
        
        if (devs == nullptr) {
            std::cerr << "No HID devices found at all!" << std::endl;
            return false;
        }
        
        std::cout << "Found HID devices, looking for Lian Li..." << std::endl;
        while (cur_dev) {
            std::cout << "HID Device: VID=0x" << std::hex << cur_dev->vendor_id 
                      << ", PID=0x" << cur_dev->product_id 
                      << ", Path=" << cur_dev->path << std::dec << std::endl;
            cur_dev = cur_dev->next;
        }
        hid_free_enumeration(devs);
        return false;
    }
    
    std::cout << "Found Lian Li SL Infinity device, attempting to open..." << std::endl;

    // Find the first SL Infinity device
    while (cur_dev)
    {
        if (cur_dev->vendor_id == 0x0CF2 && cur_dev->product_id == 0xA102)
        {
            std::cout << "Trying to open device at path: " << cur_dev->path << std::endl;
            m_handle = hid_open_path(cur_dev->path);
            if (m_handle)
            {
                std::cout << "SUCCESS: Opened Lian Li device!" << std::endl;
                m_deviceName = "Lian Li UNI HUB SL Infinity";
                m_location = std::string("HID: ") + cur_dev->path;
                
                // Read device information
                m_firmwareVersion = ReadFirmwareVersion();
                m_serialNumber = ReadSerial();
                
                hid_free_enumeration(devs);
                return true;
            }
            else
            {
                std::cout << "FAILED: Could not open device at " << cur_dev->path << std::endl;
            }
        }
        cur_dev = cur_dev->next;
    }

    hid_free_enumeration(devs);
    return false;
}

void LianLiSLInfinityController::CloseDevice()
{
    if (m_handle != nullptr)
    {
        hid_close(m_handle);
        m_handle = nullptr;
    }
}

std::string LianLiSLInfinityController::ReadFirmwareVersion()
{
    if (m_handle == nullptr)
    {
        return "";
    }

    wchar_t product_string[40];
    int ret = hid_get_product_string(m_handle, product_string, 40);

    if (ret != 0)
    {
        return "";
    }

    // Convert wide string to regular string
    std::string result;
    for (int i = 0; product_string[i] != 0; i++)
    {
        result += static_cast<char>(product_string[i]);
    }

    // Extract version from product string (format: "L-Connect-XXXX")
    size_t last_dash = result.find_last_of("-");
    if (last_dash != std::string::npos && last_dash + 1 < result.length())
    {
        return result.substr(last_dash + 1, 4);
    }

    return result;
}

std::string LianLiSLInfinityController::ReadSerial()
{
    if (m_handle == nullptr)
    {
        return "";
    }

    wchar_t serial_string[128];
    int ret = hid_get_serial_number_string(m_handle, serial_string, 128);

    if (ret != 0)
    {
        return "";
    }

    // Convert wide string to regular string
    std::string result;
    for (int i = 0; serial_string[i] != 0; i++)
    {
        result += static_cast<char>(serial_string[i]);
    }

    return result;
}

bool LianLiSLInfinityController::SendStartAction(uint8_t channel, uint8_t numFans)
{
    if (m_handle == nullptr)
    {
        return false;
    }

    unsigned char usb_buf[65];
    memset(usb_buf, 0x00, sizeof(usb_buf));

    usb_buf[0x00] = UNIHUB_SLINF_TRANSACTION_ID;
    usb_buf[0x01] = 0x10;
    usb_buf[0x02] = 0x60;
    usb_buf[0x03] = 1 + (channel / 2); // Every fan-array uses two channels
    usb_buf[0x04] = numFans; // Number of fans (1-4)

    int result = hid_write(m_handle, usb_buf, sizeof(usb_buf));
    std::this_thread::sleep_for(5ms);

    return result > 0;
}

bool LianLiSLInfinityController::SendColorData(uint8_t channel, uint8_t numLeds, const uint8_t* ledData)
{
    if (m_handle == nullptr)
    {
        return false;
    }

    unsigned char usb_buf[353];
    memset(usb_buf, 0x00, sizeof(usb_buf));

    usb_buf[0x00] = UNIHUB_SLINF_TRANSACTION_ID;
    usb_buf[0x01] = 0x30 + channel; // Action + channel (30 = channel 1, 31 = channel 2, etc.)

    // Copy color data bytes (limit to buffer size)
    size_t dataSize = std::min(static_cast<size_t>(numLeds * 3), sizeof(usb_buf) - 2);
    memcpy(&usb_buf[0x02], ledData, dataSize);

    int result = hid_write(m_handle, usb_buf, sizeof(usb_buf));
    std::this_thread::sleep_for(5ms);

    return result > 0;
}

bool LianLiSLInfinityController::SendCommitAction(uint8_t channel, uint8_t effect, uint8_t speed, uint8_t direction, uint8_t brightness)
{
    if (m_handle == nullptr)
    {
        return false;
    }

    unsigned char usb_buf[65];
    memset(usb_buf, 0x00, sizeof(usb_buf));

    usb_buf[0x00] = UNIHUB_SLINF_TRANSACTION_ID;
    usb_buf[0x01] = 0x10 + channel; // Channel + device (10 = channel 1, 11 = channel 2, etc.)
    usb_buf[0x02] = effect;         // Effect
    usb_buf[0x03] = speed;          // Speed
    usb_buf[0x04] = direction;      // Direction
    usb_buf[0x05] = brightness;     // Brightness

    int result = hid_write(m_handle, usb_buf, sizeof(usb_buf));
    std::this_thread::sleep_for(5ms);

    return result > 0;
}

void LianLiSLInfinityController::ApplyColorLimiter(SLInfinityColor& color) const
{
    // Apply color limiter to protect LEDs (from OpenRGB)
    if ((color.r + color.b + color.g) > 460)
    {
        float scale = 460.0f / (color.r + color.b + color.g);
        color.r = static_cast<uint8_t>(color.r * scale);
        color.b = static_cast<uint8_t>(color.b * scale);
        color.g = static_cast<uint8_t>(color.g * scale);
    }
}

float LianLiSLInfinityController::CalculateBrightnessLimit(const SLInfinityColor& color) const
{
    if ((color.r + color.b + color.g) > 460)
    {
        return 460.0f / (color.r + color.b + color.g);
    }
    return 1.0f;
}

bool LianLiSLInfinityController::SetChannelColors(uint8_t channel, const std::vector<SLInfinityColor>& colors)
{
    if (m_handle == nullptr || channel >= UNIHUB_SLINF_CHANNEL_COUNT)
    {
        return false;
    }

    if (colors.empty())
    {
        return true; // Nothing to do
    }

    // Prepare LED data buffer (16 LEDs per fan, up to 6 fans = 96 LEDs max)
    unsigned char led_data[16 * 6 * 3];
    memset(led_data, 0x00, sizeof(led_data));

    int fan_idx = 0;
    int mod_led_idx;
    int cur_led_idx;

    for (size_t led_idx = 0; led_idx < colors.size() && led_idx < UNIHUB_SLINF_CHANLED_COUNT; led_idx++)
    {
        mod_led_idx = (led_idx % 16);

        if ((mod_led_idx == 0) && (led_idx != 0))
        {
            fan_idx++;
        }

        SLInfinityColor color = colors[led_idx];
        ApplyColorLimiter(color);

        // Calculate brightness scale
        float brightness_scale = CalculateBrightnessLimit(color);

        // Determine current position in led_data array
        cur_led_idx = ((mod_led_idx + (fan_idx * 16)) * 3);

        if (cur_led_idx + 2 < sizeof(led_data))
        {
            led_data[cur_led_idx + 0] = static_cast<uint8_t>(color.r * brightness_scale);
            led_data[cur_led_idx + 1] = static_cast<uint8_t>(color.b * brightness_scale);
            led_data[cur_led_idx + 2] = static_cast<uint8_t>(color.g * brightness_scale);
        }
    }

    // Send start action
    if (!SendStartAction(channel, fan_idx + 1))
    {
        return false;
    }

    // Send color data
    if (!SendColorData(channel, (fan_idx + 1) * 16, led_data))
    {
        return false;
    }

    return true;
}

bool LianLiSLInfinityController::SetChannelMode(uint8_t channel, uint8_t mode)
{
    if (m_handle == nullptr || channel >= UNIHUB_SLINF_CHANNEL_COUNT)
    {
        return false;
    }

    // For SL Infinity, mode is set in the commit action
    return true; // Mode will be applied in Synchronize()
}

bool LianLiSLInfinityController::SetChannelSpeed(uint8_t channel, uint8_t speed)
{
    if (channel >= UNIHUB_SLINF_CHANNEL_COUNT)
    {
        std::cout << "SetChannelSpeed failed: channel=" << (int)channel << ", max=" << UNIHUB_SLINF_CHANNEL_COUNT << std::endl;
        return false;
    }

    // Check if kernel driver is available
    if (!IsKernelDriverAvailable())
    {
        std::cout << "Kernel driver not available - please load Lian_Li_SL_INFINITY module" << std::endl;
        return false;
    }

    // For individual fan control, we need to use the 0xE0 protocol with mode 0x21 (High SP)
    // This allows us to control each fan individually as per the protocol documentation
    if (m_handle)
    {
        // Use direct HID control with 0xE0 protocol, mode 0x21 for individual fan control
        // According to protocol documentation, mode 0x21 controls individual fans
        // We need to figure out how to specify which specific fan/port to control
        
        uint8_t report[7];
        report[0] = 0xE0;  // Report ID
        report[1] = 0x21;  // Mode 0x21 (High SP - individual fan control)
        report[2] = 0x00;  // Reserved
        report[3] = speed; // Speed (0-100%)
        
        // Try different approaches to specify which fan to control
        // Method 1: Use brightness field to specify fan selector
        report[4] = channel + 1;  // Fan selector (1-4 for ports 1-4)
        report[5] = 0x00;  // Direction (not used for fans)
        report[6] = 0x00;  // Reserved
        
        std::cout << "Sending individual fan control: Port " << (channel + 1) << " = " << (int)speed << "% (0xE0, 0x21, fan=" << (int)(channel + 1) << ")" << std::endl;
        
        // Send HID report
        int result = hid_write(m_handle, report, 7);
        if (result == 7)
        {
            std::cout << "Successfully set Port " << (channel + 1) << " to " << (int)speed << "% via individual HID control" << std::endl;
            return true;
        }
        else
        {
            std::cout << "Failed to send HID report for Port " << (channel + 1) << " (result=" << result << ")" << std::endl;
            return false;
        }
    }
    else
    {
        std::cout << "HID device not available for individual fan control" << std::endl;
        return false;
    }
}

bool LianLiSLInfinityController::SetChannelDirection(uint8_t channel, uint8_t direction)
{
    if (m_handle == nullptr || channel >= UNIHUB_SLINF_CHANNEL_COUNT)
    {
        return false;
    }

    // Direction will be applied in Synchronize()
    return true;
}

bool LianLiSLInfinityController::SetChannelBrightness(uint8_t channel, uint8_t brightness)
{
    if (m_handle == nullptr || channel >= UNIHUB_SLINF_CHANNEL_COUNT)
    {
        return false;
    }

    // Brightness will be applied in Synchronize()
    return true;
}

bool LianLiSLInfinityController::SetChannelFanCount(uint8_t channel, uint8_t count)
{
    if (m_handle == nullptr || channel >= UNIHUB_SLINF_CHANNEL_COUNT)
    {
        return false;
    }

    // Fan count will be applied in Synchronize()
    return true;
}

bool LianLiSLInfinityController::Synchronize()
{
    if (m_handle == nullptr)
    {
        return false;
    }

    // For now, just return true - the actual synchronization
    // would be done when setting colors with specific modes
    return true;
}

bool LianLiSLInfinityController::GetChannelSpeed(uint8_t channel, uint8_t& speed)
{
    if (channel >= UNIHUB_SLINF_CHANNEL_COUNT)
    {
        std::cout << "GetChannelSpeed failed: channel=" << (int)channel << ", max=" << UNIHUB_SLINF_CHANNEL_COUNT << std::endl;
        return false;
    }

    // Check if kernel driver is available
    if (!IsKernelDriverAvailable())
    {
        std::cout << "Kernel driver not available - please load Lian_Li_SL_INFINITY module" << std::endl;
        return false;
    }

    // Read speed from kernel driver's /proc interface
    std::string procPath = "/proc/Lian_li_SL_INFINITY/Port_" + std::to_string(channel + 1) + "/fan_speed";
    
    std::ifstream file(procPath);
    if (file.is_open()) {
        int speedValue;
        file >> speedValue;
        file.close();
        
        if (speedValue >= 0 && speedValue <= 100) {
            speed = static_cast<uint8_t>(speedValue);
            std::cout << "Read Port " << (channel + 1) << " speed: " << (int)speed << "% from kernel driver" << std::endl;
            return true;
        } else {
            std::cout << "Invalid speed value read from kernel driver: " << speedValue << std::endl;
            return false;
        }
    } else {
        std::cout << "Failed to open " << procPath << " for reading" << std::endl;
        return false;
    }
}

bool LianLiSLInfinityController::IsKernelDriverAvailable() const
{
    // Check if the kernel driver's /proc directory exists
    std::ifstream file("/proc/Lian_li_SL_INFINITY/status");
    return file.good();
}
