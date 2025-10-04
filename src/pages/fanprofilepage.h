#ifndef FANPROFILEPAGE_H
#define FANPROFILEPAGE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QSlider>
#include <QSpinBox>
#include <QTableWidget>
#include <QGroupBox>
#include <QRadioButton>
#include <QCheckBox>
#include <QWidget>
#include "widgets/fancurvewidget.h"
#include "usb/lian_li_sl_infinity_controller.h"

class FanProfilePage : public QWidget
{
    Q_OBJECT

public:
    explicit FanProfilePage(QWidget *parent = nullptr);

private slots:
    void onProfileChanged();
    // Button functions removed - they weren't doing anything useful
    void onStartStopToggled();
    void updateFanData();

private:
    void setupUI();
    void setupFanTable();
    void setupFanCurve();
    void setupControls();
    void updateFanCurve();
    void updateTemperature();
    void updateFanRPMs();
    void updateCPULoad();
    void updateGPULoad();
    int calculateRPMForTemperature(int temperature);
    int calculateRPMForLoad(int temperature, int cpuLoad, int gpuLoad);
    int getRealCPUTemperature();
    int getRealCPULoad();
    int getRealGPULoad();
    QVector<int> getRealFanRPMs();
    int getRealFanRPM(int port);
    int convertPercentageToRPM(int percentage);
    void controlFanSpeeds();
    void setFanSpeed(int port, int speedPercent);
    void updateFanTable();
    bool isPortConnected(int port);
    QColor getTemperatureColor(int temperature);
    // Fan detection functions removed - configuration is now in Settings
    
    QVBoxLayout *m_mainLayout;
    QVBoxLayout *m_contentLayout;
    QVBoxLayout *m_leftLayout;
    QVBoxLayout *m_rightLayout;
    
    // Fan table
    QTableWidget *m_fanTable;
    
    // Fan curve
    FanCurveWidget *m_fanCurveWidget;
    
    // Controls
    QGroupBox *m_profileGroup;
    QRadioButton *m_quietRadio;
    QRadioButton *m_stdSpRadio;
    QRadioButton *m_highSpRadio;
    QRadioButton *m_fullSpRadio;
    QRadioButton *m_customRadio;
    
    QCheckBox *m_startStopCheck;
    
    QLabel *m_tempLabel;
    QSpinBox *m_tempSpinBox;
    QLabel *m_rpmLabel;
    QSpinBox *m_rpmSpinBox;
    
    // Buttons removed - they weren't doing anything useful
    
    // Update timers
    QTimer *m_updateTimer;
    QTimer *m_tempUpdateTimer;
    QTimer *m_fanRPMTimer;
    QTimer *m_cpuLoadTimer;
    QTimer *m_gpuLoadTimer;
    
    // Cached temperature for real-time updates
    int m_cachedTemperature;
    int m_temperatureCounter;
    
    // Cached CPU and GPU load
    int m_cachedCPULoad;
    int m_cachedGPULoad;
    
    // Cached fan RPMs
    QVector<int> m_cachedFanRPMs;
    
    // Port detection
    QVector<bool> m_portConnected;
    QVector<int> m_activePorts;
    
    // HID controller for fan control
    LianLiSLInfinityController *m_hidController;
};

#endif // FANPROFILEPAGE_H
