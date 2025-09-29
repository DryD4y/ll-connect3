/*---------------------------------------------------------*\
|| lian_li_integration.h                                   |
||                                                         |
||   Integration layer for Lian Li devices in L-Connect   |
||                                                         |
||   This file is part of the L-Connect project           |
||   SPDX-License-Identifier: GPL-2.0-or-later            |
\*---------------------------------------------------------*/

#pragma once

#include "usb/lian_li_usb_controller.h"
#include <QObject>
#include <QColor>
#include <QTimer>

class LianLiIntegration : public QObject
{
    Q_OBJECT

public:
    explicit LianLiIntegration(QObject *parent = nullptr);
    ~LianLiIntegration();

    // Device management
    bool Initialize();
    void Shutdown();
    bool IsConnected() const;
    
    // Device information
    QString GetDeviceName() const;
    QString GetFirmwareVersion() const;
    QString GetSerialNumber() const;
    
    // Channel control
    bool SetChannelColor(int channel, const QColor& color);
    bool SetChannelMode(int channel, int mode);
    bool SetChannelSpeed(int channel, int speed);
    bool SetChannelBrightness(int channel, int brightness);
    bool SetChannelFanCount(int channel, int count);
    
    // Effects
    bool SetRainbowMode(int channel);
    bool SetBreathingMode(int channel, const QColor& color);
    bool SetStaticColor(int channel, const QColor& color);
    bool TurnOffChannel(int channel);
    bool TurnOffAllChannels();
    
    // Synchronization
    bool ApplyChanges();

signals:
    void deviceConnected();
    void deviceDisconnected();
    void errorOccurred(const QString& error);

private slots:
    void checkConnection();

private:
    LianLiUSBController* m_controller;
    QTimer* m_connectionTimer;
    bool m_wasConnected;
    
    // Helper methods
    LianLiColor qColorToLianLiColor(const QColor& color) const;
    int qColorToBrightness(const QColor& color) const;
};
