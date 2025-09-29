#ifndef LIGHTINGPAGE_H
#define LIGHTINGPAGE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QSlider>
#include <QGroupBox>
#include <QCheckBox>

class CustomSlider;

class LightingPage : public QWidget
{
    Q_OBJECT

public:
    explicit LightingPage(QWidget *parent = nullptr);

private slots:
    void onEffectChanged();
    void onSpeedChanged(int value);
    void onBrightnessChanged(int value);
    void onDirectionChanged();
    void onApply();
    void onMbLightingSyncToggled();

private:
    void setupUI();
    void setupControls();
    void setupProductDemo();
    void updateLightingPreview();
    
    QVBoxLayout *m_mainLayout;
    QHBoxLayout *m_headerLayout;
    QHBoxLayout *m_contentLayout;
    QVBoxLayout *m_leftLayout;
    QVBoxLayout *m_rightLayout;
    
    // Header
    QCheckBox *m_mbLightingSyncCheck;
    QPushButton *m_exportBtn;
    QPushButton *m_importBtn;
    
    // Controls
    QGroupBox *m_lightingGroup;
    QComboBox *m_effectCombo;
    CustomSlider *m_speedSlider;
    CustomSlider *m_brightnessSlider;
    
    QHBoxLayout *m_directionLayout;
    QPushButton *m_leftDirectionBtn;
    QPushButton *m_rightDirectionBtn;
    
    QPushButton *m_applyBtn;
    
    // Product demo
    QLabel *m_demoLabel;
    QComboBox *m_productCombo;
    QLabel *m_demoImageLabel;
    
    // Current settings
    QString m_currentEffect;
    int m_currentSpeed;
    int m_currentBrightness;
    bool m_directionLeft;
};

#endif // LIGHTINGPAGE_H
