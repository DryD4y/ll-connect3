#ifndef SETTINGSPAGE_H
#define SETTINGSPAGE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QGroupBox>
#include <QLineEdit>

class SettingsPage : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsPage(QWidget *parent = nullptr);

private slots:
    void onAutoRunToggled();
    void onHideOnTrayToggled();
    void onMinimizeToTrayToggled();
    void onFloatingWindowToggled();
    void onLanguageChanged();
    void onTemperatureUnitChanged();
    void onServiceDelayChanged();
    void onApplyServiceDelay();
    void onResetAll();
    void onFanPortToggled(int port, bool enabled);

private:
    void setupUI();
    void setupGeneralSettings();
    void setupServiceSettings();
    void setupFanConfiguration();
    void setupDebugSettings();
    void applySettings();
    void loadFanConfiguration();
    void saveFanConfiguration();
    
    QVBoxLayout *m_mainLayout;
    QHBoxLayout *m_headerLayout;
    QHBoxLayout *m_contentLayout;
    QVBoxLayout *m_leftLayout;
    QVBoxLayout *m_rightLayout;
    
    // Header
    QLabel *m_titleLabel;
    QLabel *m_floatingWindowLabel;
    QFrame *m_toggleFrame;
    
    // General settings
    QGroupBox *m_generalGroup;
    QCheckBox *m_autoRunCheck;
    QCheckBox *m_hideOnTrayCheck;
    QCheckBox *m_minimizeToTrayCheck;
    QCheckBox *m_floatingWindowCheck;
    
    QComboBox *m_languageCombo;
    QComboBox *m_temperatureCombo;
    
    // Service settings
    QGroupBox *m_serviceGroup;
    QPushButton *m_serviceButton;
    QLabel *m_delayLabel;
    QSpinBox *m_delaySpinBox;
    QPushButton *m_applyDelayBtn;
    
    // Fan configuration
    QGroupBox *m_fanConfigGroup;
    QCheckBox *m_fanPort1Check;
    QCheckBox *m_fanPort2Check;
    QCheckBox *m_fanPort3Check;
    QCheckBox *m_fanPort4Check;
    
    // Debug settings
    QGroupBox *m_debugGroup;
    QCheckBox *m_debugModeCheck;
    
    // Action buttons
    QPushButton *m_resetAllBtn;
    QPushButton *m_applyBtn;
};

#endif // SETTINGSPAGE_H
