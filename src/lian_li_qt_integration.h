/*---------------------------------------------------------*\
|| lian_li_qt_integration.h                               |
||                                                         |
||   Qt Integration for Lian Li SL Infinity Controller    |
||   Provides easy-to-use Qt interface for RGB control    |
||                                                         |
||   This file is part of the L-Connect project           |
||   SPDX-License-Identifier: GPL-2.0-or-later            |
\*---------------------------------------------------------*/

#pragma once

#include <QObject>
#include <QColor>
#include <QTimer>
#include <QString>
#include <memory>
#include "usb/sl_infinity_hid.h"

class LianLiQtIntegration : public QObject
{
    Q_OBJECT

public:
    explicit LianLiQtIntegration(QObject *parent = nullptr);
    ~LianLiQtIntegration();

    // Device management
    bool initialize();
    void shutdown();
    bool isConnected() const;
    
    // Device information
    QString getDeviceName() const;
    QString getFirmwareVersion() const;
    QString getSerialNumber() const;
    
    // RGB Control
    bool setChannelColor(int channel, const QColor &color);
    bool setChannelMode(int channel, int mode);
    bool turnOffChannel(int channel);
    bool turnOffAllChannels();
    
    // Convenience methods
    bool setAllChannelsColor(const QColor &color);
    bool setRainbowEffect();
    bool setBreathingEffect(const QColor &color);
    
    // Channel management
    int getChannelCount() const { return 8; }
    bool isChannelValid(int channel) const { return channel >= 0 && channel < 8; }

signals:
    void deviceConnected();
    void deviceDisconnected();
    void errorOccurred(const QString &error);
    void colorChanged(int channel, const QColor &color);

private slots:
    void onDeviceCheck();

private:
    std::unique_ptr<SLInfinityHIDController> m_controller;
    QTimer *m_deviceCheckTimer;
    bool m_wasConnected;
    
    // Helper methods
    SLInfinityColor qColorToSLInfinity(const QColor &color) const;
    QColor slInfinityToQColor(const SLInfinityColor &color) const;
};
