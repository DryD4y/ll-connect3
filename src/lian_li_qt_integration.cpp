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

bool LianLiQtIntegration::setChannelColor(int channel, const QColor &color)
{
    if (!isChannelValid(channel) || !isConnected()) {
        return false;
    }
    
    SLInfinityColor slColor = qColorToSLInfinity(color);
    std::vector<SLInfinityColor> colors = {slColor};
    
    bool success = m_controller->SetChannelColors(static_cast<uint8_t>(channel), colors);
    
    if (success) {
        // Send commit action for static color mode
        success = m_controller->SendCommitAction(
            static_cast<uint8_t>(channel), 
            0x01, // Static color mode
            0x00, // 50% speed
            0x00, // Left-to-right direction
            0x00  // 100% brightness
        );
        
        if (success) {
            emit colorChanged(channel, color);
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

bool LianLiQtIntegration::setAllChannelsColor(const QColor &color)
{
    if (!isConnected()) {
        return false;
    }
    
    bool allSuccess = true;
    for (int channel = 0; channel < getChannelCount(); channel++) {
        if (!setChannelColor(channel, color)) {
            allSuccess = false;
        }
    }
    
    return allSuccess;
}

bool LianLiQtIntegration::setRainbowEffect()
{
    if (!isConnected()) {
        return false;
    }
    
    bool allSuccess = true;
    for (int channel = 0; channel < getChannelCount(); channel++) {
        if (!isChannelValid(channel)) continue;
        
        // Send commit action for rainbow effect
        bool success = m_controller->SendCommitAction(
            static_cast<uint8_t>(channel),
            0x05, // Rainbow mode
            0x00, // 50% speed
            0x00, // Left-to-right direction
            0x00  // 100% brightness
        );
        
        if (!success) {
            allSuccess = false;
        }
    }
    
    return allSuccess;
}

bool LianLiQtIntegration::setBreathingEffect(const QColor &color)
{
    if (!isConnected()) {
        return false;
    }
    
    // Set the color first
    if (!setAllChannelsColor(color)) {
        return false;
    }
    
    // Then set breathing mode
    bool allSuccess = true;
    for (int channel = 0; channel < getChannelCount(); channel++) {
        if (!isChannelValid(channel)) continue;
        
        bool success = m_controller->SendCommitAction(
            static_cast<uint8_t>(channel),
            0x02, // Breathing mode
            0x00, // 50% speed
            0x00, // Left-to-right direction
            0x00  // 100% brightness
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
