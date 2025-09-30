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
    void onApplyToAll();
    void onDefault();
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
    int calculateRPMForTemperature(int temperature);
    int getRealCPUTemperature();
    QVector<int> getRealFanRPMs();
    void controlFanSpeeds();
    void setFanSpeed(int port, int speedPercent);
    
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
    
    QPushButton *m_applyToAllBtn;
    QPushButton *m_defaultBtn;
    
    // Update timers
    QTimer *m_updateTimer;
    QTimer *m_tempUpdateTimer;
    QTimer *m_fanRPMTimer;
    
    // Cached temperature for real-time updates
    int m_cachedTemperature;
    int m_temperatureCounter;
    
    // Cached fan RPMs
    QVector<int> m_cachedFanRPMs;
    
    // HID controller for fan control
    LianLiSLInfinityController *m_hidController;
};

#endif // FANPROFILEPAGE_H
