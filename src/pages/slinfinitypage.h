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
    void onEffectChanged();
    void onSpeedChanged(int value);
    void onBrightnessChanged(int value);
    void onDirectionChanged();
    void onApplyToAll();
    void onDefault();
    void onColorButtonClicked();
    void onDeviceConnected();
    void onDeviceDisconnected();

private:
    void setupUI();
    void setupFanVisualization();
    void setupControls();
    void updateFanVisualization();
    void applyCurrentEffect();
    void selectPort(int port);
    void updatePortSelection();
    void saveLightingSettings();
    void loadLightingSettings();
    void clearOldEffectSettings(const QString &oldEffect, const QString &newEffect);
    
    QVBoxLayout *m_mainLayout;
    QHBoxLayout *m_headerLayout;
    QHBoxLayout *m_contentLayout;
    QVBoxLayout *m_leftLayout;
    QVBoxLayout *m_rightLayout;
    
    // Header
    QLabel *m_controllerLabel;
    
    // Fan visualization
    QGroupBox *m_fanGroup;
    QGridLayout *m_fanGrid;
    FanWidget *m_fanWidgets[4][4]; // 4 ports, 4 fans each
    
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
    
    // Selected port(s) - -1 means none selected, single port 0-3, or use QSet for multiple
    int m_selectedPort; // -1 = none, 0-3 = port index
    
    // Color storage - per port (4 ports, up to 3 colors per effect)
    QColor m_portColors[4][4]; // [port][color_index] - supports up to 4 colors per port
    
    // Lian Li integration
    LianLiQtIntegration *m_lianLi;
    QPushButton *m_colorButton;
    QPushButton *m_colorButtons[4]; // For multiple color selection (Tide: 2, ColorCycle: 3)
    QLabel *m_colorLabel;
    QLabel *m_directionLabel;
    QLabel *m_statusLabel;
    QTimer *m_statusTimer;
    QWidget *m_multiColorWidget; // Container for multiple color buttons
};

#endif // SLINFINITYPAGE_H
