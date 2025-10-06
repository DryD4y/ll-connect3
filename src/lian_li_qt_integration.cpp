/*---------------------------------------------------------*\
|| lian_li_qt_integration.cpp                             |
||                                                         |
||   Qt Integration for Lian Li SL Infinity Controller    |
||   Provides easy-to-use Qt interface for RGB control    |
||                                                         |
||   This file is part of the L-Connect project           |
||   SPDX-License-Identifier: GPL-2.0-or-later            |
\*---------------------------------------------------------*/

#include "lian_li_qt_integration.h"
#include <QDebug>
#include <QThread>
#include <QApplication>

LianLiQtIntegration::LianLiQtIntegration(QObject *parent)
    : QObject(parent)
    , m_controller(std::make_unique<SLInfinityHIDController>())
    , m_deviceCheckTimer(new QTimer(this))
    , m_wasConnected(false)
{
    // Set up device monitoring timer
    m_deviceCheckTimer->setInterval(2000); // Check every 2 seconds
    connect(m_deviceCheckTimer, &QTimer::timeout, this, &LianLiQtIntegration::onDeviceCheck);
}

LianLiQtIntegration::~LianLiQtIntegration()
{
    shutdown();
}

bool LianLiQtIntegration::initialize()
{
    if (!m_controller) {
        m_controller = std::make_unique<SLInfinityHIDController>();
    }
    
    if (m_controller->Initialize()) {
        m_wasConnected = true;
        m_deviceCheckTimer->start();
        emit deviceConnected();
        qDebug() << "Lian Li device connected successfully";
        return true;
    } else {
        emit errorOccurred("Failed to initialize Lian Li device");
        qDebug() << "Failed to initialize Lian Li device";
        return false;
    }
}

void LianLiQtIntegration::shutdown()
{
    if (m_deviceCheckTimer) {
        m_deviceCheckTimer->stop();
    }
    
    if (m_controller) {
        m_controller->Close();
        m_controller.reset();
    }
    
    m_wasConnected = false;
}

bool LianLiQtIntegration::isConnected() const
{
    return m_controller && m_controller->IsConnected();
}

QString LianLiQtIntegration::getDeviceName() const
{
    if (!m_controller) return "Unknown";
    return QString::fromStdString(m_controller->GetDeviceName());
}

QString LianLiQtIntegration::getFirmwareVersion() const
{
    if (!m_controller) return "Unknown";
    return QString::fromStdString(m_controller->GetFirmwareVersion());
}

QString LianLiQtIntegration::getSerialNumber() const
{
    if (!m_controller) return "Unknown";
    return QString::fromStdString(m_controller->GetSerialNumber());
}

bool LianLiQtIntegration::setChannelColor(int channel, const QColor &color, int brightness)
{
    qDebug() << "setChannelColor: channel=" << channel << "color=" << color << "brightness=" << brightness;
    
    if (!isChannelValid(channel) || !isConnected()) {
        qDebug() << "setChannelColor: Invalid channel or not connected. Channel valid:" << isChannelValid(channel) << "Connected:" << isConnected();
        return false;
    }
    
    SLInfinityColor slColor = qColorToSLInfinity(color);
    
    // For static color, we need to set ALL 16 LEDs per fan to the same color
    // Each channel can have up to 4 fans, so we need 16 * 4 = 64 LEDs total
    std::vector<SLInfinityColor> colors;
    colors.resize(64, slColor); // Fill all 64 LEDs with the same color
    
    uint8_t hwBrightness = convertBrightness(brightness);
    qDebug() << "Brightness conversion: input=" << brightness << "output=" << hwBrightness;
    
    bool success = m_controller->SetChannelColors(static_cast<uint8_t>(channel), colors);
    qDebug() << "SetChannelColors for channel" << channel << "result:" << success;
    
    if (success) {
        // Set the channel mode to static color
        success = m_controller->SetChannelMode(static_cast<uint8_t>(channel), 0x01);
        qDebug() << "SetChannelMode for channel" << channel << "result:" << success;
        
        if (success) {
            // Send commit action for static color mode with brightness
            success = m_controller->SendCommitAction(
                static_cast<uint8_t>(channel), 
                0x01, // Static color mode
                0x00, // Speed doesn't matter for static
                0x00, // Direction doesn't matter for static
                hwBrightness  // Use the actual brightness value
            );
            qDebug() << "SendCommitAction for channel" << channel << "result:" << success;
            
            if (success) {
                emit colorChanged(channel, color);
            }
        }
    }
    
    return success;
}

bool LianLiQtIntegration::setChannelMode(int channel, int mode)
{
    if (!isChannelValid(channel) || !isConnected()) {
        return false;
    }
    
    return m_controller->SetChannelMode(static_cast<uint8_t>(channel), static_cast<uint8_t>(mode));
}

bool LianLiQtIntegration::turnOffChannel(int channel)
{
    if (!isChannelValid(channel) || !isConnected()) {
        return false;
    }
    
    bool success = m_controller->TurnOffChannel(static_cast<uint8_t>(channel));
    
    if (success) {
        emit colorChanged(channel, QColor(0, 0, 0));
    }
    
    return success;
}

bool LianLiQtIntegration::turnOffAllChannels()
{
    if (!isConnected()) {
        return false;
    }
    
    bool success = m_controller->TurnOffAllChannels();
    
    if (success) {
        for (int i = 0; i < getChannelCount(); i++) {
            emit colorChanged(i, QColor(0, 0, 0));
        }
    }
    
    return success;
}

bool LianLiQtIntegration::setAllChannelsColor(const QColor &color, int brightness)
{
    if (!isConnected()) {
        return false;
    }
    
    SLInfinityColor slColor = qColorToSLInfinity(color);
    
    // For static color, we need to set ALL 16 LEDs per fan to the same color
    // Each channel can have up to 4 fans, so we need 16 * 4 = 64 LEDs total
    std::vector<SLInfinityColor> colors;
    colors.resize(64, slColor); // Fill all 64 LEDs with the same color
    
    uint8_t hwBrightness = convertBrightness(brightness);
    
    bool allSuccess = true;
    for (int channel = 0; channel < getChannelCount(); channel++) {
        if (!isChannelValid(channel)) continue;
        
        // Set the color
        if (!m_controller->SetChannelColors(static_cast<uint8_t>(channel), colors)) {
            allSuccess = false;
            continue;
        }
        
        // Send commit action for static color mode with brightness
        if (!m_controller->SendCommitAction(
            static_cast<uint8_t>(channel), 
            0x01, // Static color mode
            0x00, // Speed doesn't matter for static
            0x00, // Direction doesn't matter for static
            hwBrightness)) {
            allSuccess = false;
        }
    }
    
    return allSuccess;
}

bool LianLiQtIntegration::setRainbowEffect(int speed, int brightness, bool directionLeft)
{
    if (!isConnected()) {
        return false;
    }
    
    uint8_t hwSpeed = convertSpeed(speed);
    uint8_t hwBrightness = convertBrightness(brightness);
    uint8_t hwDirection = convertDirection(directionLeft);
    
    bool allSuccess = true;
    for (int channel = 0; channel < getChannelCount(); channel++) {
        if (!isChannelValid(channel)) continue;
        
        // Send commit action for rainbow effect
        bool success = m_controller->SendCommitAction(
            static_cast<uint8_t>(channel),
            0x05, // Rainbow mode
            hwSpeed,
            hwDirection,
            hwBrightness
        );
        
        if (!success) {
            allSuccess = false;
        }
    }
    
    return allSuccess;
}

bool LianLiQtIntegration::setRainbowMorphEffect(int speed, int brightness, bool directionLeft)
{
    if (!isConnected()) {
        return false;
    }
    
    uint8_t hwSpeed = convertSpeed(speed);
    uint8_t hwBrightness = convertBrightness(brightness);
    uint8_t hwDirection = convertDirection(directionLeft);
    
    bool allSuccess = true;
    for (int channel = 0; channel < getChannelCount(); channel++) {
        if (!isChannelValid(channel)) continue;
        
        // Send commit action for rainbow morph effect
        bool success = m_controller->SendCommitAction(
            static_cast<uint8_t>(channel),
            0x04, // Rainbow Morph mode
            hwSpeed,
            hwDirection,
            hwBrightness
        );
        
        if (!success) {
            allSuccess = false;
        }
    }
    
    return allSuccess;
}

bool LianLiQtIntegration::setMeteorEffect(int speed, int brightness, bool directionLeft)
{
    if (!isConnected()) {
        return false;
    }
    
    uint8_t hwSpeed = convertSpeed(speed);
    uint8_t hwBrightness = convertBrightness(brightness);
    uint8_t hwDirection = convertDirection(directionLeft);
    
    bool allSuccess = true;
    for (int channel = 0; channel < getChannelCount(); channel++) {
        if (!isChannelValid(channel)) continue;
        
        // Send commit action for meteor effect
        bool success = m_controller->SendCommitAction(
            static_cast<uint8_t>(channel),
            0x24, // Meteor mode
            hwSpeed,
            hwDirection,
            hwBrightness
        );
        
        if (!success) {
            allSuccess = false;
        }
    }
    
    return allSuccess;
}

bool LianLiQtIntegration::setRunwayEffect(int speed, int brightness, bool directionLeft)
{
    if (!isConnected()) {
        return false;
    }
    
    uint8_t hwSpeed = convertSpeed(speed);
    uint8_t hwBrightness = convertBrightness(brightness);
    uint8_t hwDirection = convertDirection(directionLeft);
    
    bool allSuccess = true;
    for (int channel = 0; channel < getChannelCount(); channel++) {
        if (!isChannelValid(channel)) continue;
        
        // Send commit action for runway effect
        bool success = m_controller->SendCommitAction(
            static_cast<uint8_t>(channel),
            0x1C, // Runway mode
            hwSpeed,
            hwDirection,
            hwBrightness
        );
        
        if (!success) {
            allSuccess = false;
        }
    }
    
    return allSuccess;
}

bool LianLiQtIntegration::setBreathingEffect(const QColor &color, int speed, int brightness, bool directionLeft)
{
    if (!isConnected()) {
        return false;
    }
    
    SLInfinityColor slColor = qColorToSLInfinity(color);
    std::vector<SLInfinityColor> colors = {slColor};
    uint8_t hwSpeed = convertSpeed(speed);
    uint8_t hwBrightness = convertBrightness(brightness);
    uint8_t hwDirection = convertDirection(directionLeft);
    
    bool allSuccess = true;
    for (int channel = 0; channel < getChannelCount(); channel++) {
        if (!isChannelValid(channel)) continue;
        
        // Set the color
        if (!m_controller->SetChannelColors(static_cast<uint8_t>(channel), colors)) {
            allSuccess = false;
            continue;
        }
        
        // Send commit action for breathing mode
        bool success = m_controller->SendCommitAction(
            static_cast<uint8_t>(channel),
            0x02, // Breathing mode
            hwSpeed,
            hwDirection,
            hwBrightness
        );
        
        if (!success) {
            allSuccess = false;
        }
    }
    
    return allSuccess;
}

void LianLiQtIntegration::onDeviceCheck()
{
    bool currentlyConnected = isConnected();
    
    if (currentlyConnected != m_wasConnected) {
        m_wasConnected = currentlyConnected;
        
        if (currentlyConnected) {
            emit deviceConnected();
            qDebug() << "Lian Li device connected";
        } else {
            emit deviceDisconnected();
            qDebug() << "Lian Li device disconnected";
        }
    }
}

SLInfinityColor LianLiQtIntegration::qColorToSLInfinity(const QColor &color) const
{
    return SLInfinityColor::fromRGB(
        static_cast<uint8_t>(color.red()),
        static_cast<uint8_t>(color.green()),
        static_cast<uint8_t>(color.blue())
    );
}

QColor LianLiQtIntegration::slInfinityToQColor(const SLInfinityColor &color) const
{
    return QColor(color.r, color.g, color.b);
}

uint8_t LianLiQtIntegration::convertSpeed(int speedPercent)
{
    // Convert 0-100% to hardware values (25% increments only)
    // Hardware values from lian_li_sl_infinity_controller.h:
    // UNIHUB_SLINF_LED_SPEED_000 = 0x02, SPEED_025 = 0x01, SPEED_050 = 0x00
    // UNIHUB_SLINF_LED_SPEED_075 = 0xFF, SPEED_100 = 0xFE
    if (speedPercent <= 12) return 0x02;       // 0-12% → 0% (very slow)
    else if (speedPercent <= 37) return 0x01;  // 13-37% → 25% (slow)
    else if (speedPercent <= 62) return 0x00;  // 38-62% → 50% (medium)
    else if (speedPercent <= 87) return 0xFF;  // 63-87% → 75% (fast)
    else return 0xFE;                          // 88-100% → 100% (very fast)
}

uint8_t LianLiQtIntegration::convertBrightness(int brightnessPercent)
{
    // Convert 0-100% to hardware values (25% increments only)
    // Hardware values from lian_li_sl_infinity_controller.h:
    // UNIHUB_SLINF_LED_BRIGHTNESS_000 = 0x08, BRIGHTNESS_025 = 0x03, BRIGHTNESS_050 = 0x02
    // UNIHUB_SLINF_LED_BRIGHTNESS_075 = 0x01, BRIGHTNESS_100 = 0x00
    if (brightnessPercent <= 12) return 0x08;       // 0-12% → 0% (off)
    else if (brightnessPercent <= 37) return 0x03;  // 13-37% → 25% (dim)
    else if (brightnessPercent <= 62) return 0x02;  // 38-62% → 50% (medium)
    else if (brightnessPercent <= 87) return 0x01;  // 63-87% → 75% (bright)
    else return 0x00;                               // 88-100% → 100% (full)
}

uint8_t LianLiQtIntegration::convertDirection(bool directionLeft)
{
    // Hardware: 0x00 (left-to-right), 0x01 (right-to-left)
    return directionLeft ? 0x01 : 0x00;
}
