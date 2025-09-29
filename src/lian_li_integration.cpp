/*---------------------------------------------------------*\
|| lian_li_integration.cpp                                 |
||                                                         |
||   Integration layer for Lian Li devices in L-Connect   |
||                                                         |
||   This file is part of the L-Connect project           |
||   SPDX-License-Identifier: GPL-2.0-or-later            |
\*---------------------------------------------------------*/

#include "lian_li_integration.h"
#include <QDebug>

LianLiIntegration::LianLiIntegration(QObject *parent)
    : QObject(parent)
    , m_controller(nullptr)
    , m_connectionTimer(new QTimer(this))
    , m_wasConnected(false)
{
    // Set up connection monitoring timer
    m_connectionTimer->setInterval(1000); // Check every second
    connect(m_connectionTimer, &QTimer::timeout, this, &LianLiIntegration::checkConnection);
}

LianLiIntegration::~LianLiIntegration()
{
    Shutdown();
}

bool LianLiIntegration::Initialize()
{
    if (m_controller)
    {
        return m_controller->IsConnected();
    }

    m_controller = new LianLiUSBController();
    
    if (!m_controller->Initialize())
    {
        delete m_controller;
        m_controller = nullptr;
        emit errorOccurred("Failed to initialize Lian Li device");
        return false;
    }

    m_wasConnected = true;
    emit deviceConnected();
    
    // Start connection monitoring
    m_connectionTimer->start();
    
    qDebug() << "Lian Li device connected:" << GetDeviceName();
    return true;
}

void LianLiIntegration::Shutdown()
{
    if (m_connectionTimer)
    {
        m_connectionTimer->stop();
    }

    if (m_controller)
    {
        TurnOffAllChannels();
        delete m_controller;
        m_controller = nullptr;
    }
    
    if (m_wasConnected)
    {
        m_wasConnected = false;
        emit deviceDisconnected();
    }
}

bool LianLiIntegration::IsConnected() const
{
    return m_controller && m_controller->IsConnected();
}

QString LianLiIntegration::GetDeviceName() const
{
    if (!m_controller)
    {
        return QString();
    }
    return QString::fromStdString(m_controller->GetDeviceName());
}

QString LianLiIntegration::GetFirmwareVersion() const
{
    if (!m_controller)
    {
        return QString();
    }
    return QString::fromStdString(m_controller->GetFirmwareVersion());
}

QString LianLiIntegration::GetSerialNumber() const
{
    if (!m_controller)
    {
        return QString();
    }
    return QString::fromStdString(m_controller->GetSerialNumber());
}

bool LianLiIntegration::SetChannelColor(int channel, const QColor& color)
{
    if (!m_controller || !IsConnected())
    {
        return false;
    }

    LianLiColor lianLiColor = qColorToLianLiColor(color);
    std::vector<LianLiColor> colors = {lianLiColor};
    
    return m_controller->SetChannelColors(channel, colors);
}

bool LianLiIntegration::SetChannelMode(int channel, int mode)
{
    if (!m_controller || !IsConnected())
    {
        return false;
    }

    return m_controller->SetChannelMode(channel, mode);
}

bool LianLiIntegration::SetChannelSpeed(int channel, int speed)
{
    if (!m_controller || !IsConnected())
    {
        return false;
    }

    return m_controller->SetChannelSpeed(channel, speed);
}

bool LianLiIntegration::SetChannelBrightness(int channel, int brightness)
{
    if (!m_controller || !IsConnected())
    {
        return false;
    }

    return m_controller->SetChannelBrightness(channel, brightness);
}

bool LianLiIntegration::SetChannelFanCount(int channel, int count)
{
    if (!m_controller || !IsConnected())
    {
        return false;
    }

    return m_controller->SetChannelFanCount(channel, count);
}

bool LianLiIntegration::SetRainbowMode(int channel)
{
    if (!m_controller || !IsConnected())
    {
        return false;
    }

    return m_controller->SetChannelMode(channel, UNIHUB_LED_MODE_RAINBOW) &&
           m_controller->SetChannelSpeed(channel, UNIHUB_LED_SPEED_050) &&
           m_controller->SetChannelBrightness(channel, UNIHUB_LED_BRIGHTNESS_100);
}

bool LianLiIntegration::SetBreathingMode(int channel, const QColor& color)
{
    if (!m_controller || !IsConnected())
    {
        return false;
    }

    LianLiColor lianLiColor = qColorToLianLiColor(color);
    std::vector<LianLiColor> colors = {lianLiColor};
    
    return m_controller->SetChannelMode(channel, UNIHUB_LED_MODE_BREATHING) &&
           m_controller->SetChannelColors(channel, colors) &&
           m_controller->SetChannelSpeed(channel, UNIHUB_LED_SPEED_025) &&
           m_controller->SetChannelBrightness(channel, UNIHUB_LED_BRIGHTNESS_100);
}

bool LianLiIntegration::SetStaticColor(int channel, const QColor& color)
{
    if (!m_controller || !IsConnected())
    {
        return false;
    }

    LianLiColor lianLiColor = qColorToLianLiColor(color);
    std::vector<LianLiColor> colors = {lianLiColor};
    
    return m_controller->SetChannelMode(channel, UNIHUB_LED_MODE_STATIC_COLOR) &&
           m_controller->SetChannelColors(channel, colors) &&
           m_controller->SetChannelBrightness(channel, qColorToBrightness(color));
}

bool LianLiIntegration::TurnOffChannel(int channel)
{
    if (!m_controller || !IsConnected())
    {
        return false;
    }

    LianLiColor blackColor = LianLiColor::fromRGB(0, 0, 0);
    std::vector<LianLiColor> colors = {blackColor};
    
    return m_controller->SetChannelMode(channel, UNIHUB_LED_MODE_STATIC_COLOR) &&
           m_controller->SetChannelColors(channel, colors) &&
           m_controller->SetChannelBrightness(channel, UNIHUB_LED_BRIGHTNESS_000);
}

bool LianLiIntegration::TurnOffAllChannels()
{
    if (!m_controller || !IsConnected())
    {
        return false;
    }

    bool success = true;
    for (int channel = 0; channel < 4; channel++)
    {
        success &= TurnOffChannel(channel);
    }
    
    return success;
}

bool LianLiIntegration::ApplyChanges()
{
    if (!m_controller || !IsConnected())
    {
        return false;
    }

    return m_controller->Synchronize();
}

void LianLiIntegration::checkConnection()
{
    bool currentlyConnected = IsConnected();
    
    if (currentlyConnected != m_wasConnected)
    {
        m_wasConnected = currentlyConnected;
        
        if (currentlyConnected)
        {
            emit deviceConnected();
            qDebug() << "Lian Li device reconnected";
        }
        else
        {
            emit deviceDisconnected();
            qDebug() << "Lian Li device disconnected";
        }
    }
}

LianLiColor LianLiIntegration::qColorToLianLiColor(const QColor& color) const
{
    return LianLiColor::fromRGB(
        static_cast<uint8_t>(color.red()),
        static_cast<uint8_t>(color.green()),
        static_cast<uint8_t>(color.blue())
    );
}

int LianLiIntegration::qColorToBrightness(const QColor& color) const
{
    // Convert QColor brightness to Lian Li brightness levels
    int brightness = color.lightness();
    
    if (brightness <= 32) return UNIHUB_LED_BRIGHTNESS_000;
    if (brightness <= 64) return UNIHUB_LED_BRIGHTNESS_025;
    if (brightness <= 128) return UNIHUB_LED_BRIGHTNESS_050;
    if (brightness <= 192) return UNIHUB_LED_BRIGHTNESS_075;
    return UNIHUB_LED_BRIGHTNESS_100;
}
