#ifndef SLINFINITYPAGE_H
#define SLINFINITYPAGE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QGroupBox>
#include <QSlider>
#include <QSpinBox>
#include <QColorDialog>
#include <QTimer>

class FanWidget;
class LianLiQtIntegration;

class SLInfinityPage : public QWidget
{
    Q_OBJECT

public:
    explicit SLInfinityPage(QWidget *parent = nullptr);

private slots:
    void onProfileChanged();
    void onEffectChanged();
    void onSpeedChanged(int value);
    void onBrightnessChanged(int value);
    void onDirectionChanged();
    void onApplyToAll();
    void onDefault();
    void onExport();
    void onImport();
    void onColorButtonClicked();
    void onDeviceConnected();
    void onDeviceDisconnected();

private:
    void setupUI();
    void setupFanVisualization();
    void setupControls();
    void updateFanVisualization();
    
    QVBoxLayout *m_mainLayout;
    QHBoxLayout *m_headerLayout;
    QHBoxLayout *m_contentLayout;
    QVBoxLayout *m_leftLayout;
    QVBoxLayout *m_rightLayout;
    
    // Header
    QLabel *m_controllerLabel;
    QPushButton *m_exportBtn;
    QPushButton *m_importBtn;
    
    // Fan visualization
    QGroupBox *m_fanGroup;
    QGridLayout *m_fanGrid;
    FanWidget *m_fanWidgets[4][4]; // 4 ports, 4 fans each
    
    // Fan profile controls
    QGroupBox *m_profileGroup;
    QComboBox *m_profileCombo;
    QPushButton *m_applyToAllBtn;
    
    // Lighting controls
    QGroupBox *m_lightingGroup;
    QComboBox *m_effectCombo;
    QSlider *m_speedSlider;
    QSlider *m_brightnessSlider;
    QHBoxLayout *m_directionLayout;
    QPushButton *m_leftDirectionBtn;
    QPushButton *m_rightDirectionBtn;
    QPushButton *m_applyToAllLightingBtn;
    QPushButton *m_defaultBtn;
    
    // Current settings
    QString m_currentProfile;
    QString m_currentEffect;
    int m_currentSpeed;
    int m_currentBrightness;
    bool m_directionLeft;
    
    // Lian Li integration
    LianLiQtIntegration *m_lianLi;
    QPushButton *m_colorButton;
    QLabel *m_statusLabel;
    QTimer *m_statusTimer;
};

#endif // SLINFINITYPAGE_H
