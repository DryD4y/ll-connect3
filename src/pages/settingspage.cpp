#include "settingspage.h"

SettingsPage::SettingsPage(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
    setupGeneralSettings();
    setupServiceSettings();
    applySettings();
}

void SettingsPage::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(20, 20, 20, 20);
    m_mainLayout->setSpacing(20);
    
    // Header
    m_headerLayout = new QHBoxLayout();
    m_headerLayout->setContentsMargins(0, 0, 0, 0);
    
    m_titleLabel = new QLabel("General");
    m_titleLabel->setObjectName("pageTitle");
    
    m_floatingWindowLabel = new QLabel("Displays a floating system information window");
    m_floatingWindowLabel->setObjectName("floatingWindowLabel");
    
    m_toggleFrame = new QFrame();
    m_toggleFrame->setObjectName("toggleFrame");
    m_toggleFrame->setFixedSize(50, 24);
    
    m_headerLayout->addWidget(m_titleLabel);
    m_headerLayout->addStretch();
    m_headerLayout->addWidget(m_floatingWindowLabel);
    m_headerLayout->addWidget(m_toggleFrame);
    
    m_mainLayout->addLayout(m_headerLayout);
    
    // Content layout
    m_contentLayout = new QHBoxLayout();
    m_contentLayout->setSpacing(20);
    
    m_leftLayout = new QVBoxLayout();
    m_rightLayout = new QVBoxLayout();
    
    m_contentLayout->addLayout(m_leftLayout, 1);
    m_contentLayout->addLayout(m_rightLayout, 1);
    
    m_mainLayout->addLayout(m_contentLayout);
    
    // Apply styles
    setStyleSheet(R"(
        #pageTitle {
            color: #ffffff;
            font-size: 24px;
            font-weight: bold;
        }
        
        #floatingWindowLabel {
            color: #cccccc;
            font-size: 14px;
        }
        
        #toggleFrame {
            background-color: #2a82da;
            border-radius: 12px;
            border: none;
        }
    )");
}

void SettingsPage::setupGeneralSettings()
{
    m_generalGroup = new QGroupBox("General Settings");
    m_generalGroup->setObjectName("settingsGroup");
    
    QVBoxLayout *generalLayout = new QVBoxLayout(m_generalGroup);
    generalLayout->setSpacing(15);
    
    // Auto-run checkbox
    m_autoRunCheck = new QCheckBox("Auto-Run on boot");
    m_autoRunCheck->setObjectName("settingsCheck");
    m_autoRunCheck->setChecked(true);
    connect(m_autoRunCheck, &QCheckBox::toggled, this, &SettingsPage::onAutoRunToggled);
    
    // Hide on tray checkbox
    m_hideOnTrayCheck = new QCheckBox("Hide on system tray when auto run on boot");
    m_hideOnTrayCheck->setObjectName("settingsCheck");
    m_hideOnTrayCheck->setChecked(true);
    connect(m_hideOnTrayCheck, &QCheckBox::toggled, this, &SettingsPage::onHideOnTrayToggled);
    
    // Minimize to tray checkbox
    m_minimizeToTrayCheck = new QCheckBox("Minimize L-Connect 3 to system tray at close");
    m_minimizeToTrayCheck->setObjectName("settingsCheck");
    m_minimizeToTrayCheck->setChecked(true);
    connect(m_minimizeToTrayCheck, &QCheckBox::toggled, this, &SettingsPage::onMinimizeToTrayToggled);
    
    // Floating window checkbox
    m_floatingWindowCheck = new QCheckBox("Displays a floating system information window");
    m_floatingWindowCheck->setObjectName("settingsCheck");
    m_floatingWindowCheck->setChecked(true);
    connect(m_floatingWindowCheck, &QCheckBox::toggled, this, &SettingsPage::onFloatingWindowToggled);
    
    generalLayout->addWidget(m_autoRunCheck);
    generalLayout->addWidget(m_hideOnTrayCheck);
    generalLayout->addWidget(m_minimizeToTrayCheck);
    generalLayout->addWidget(m_floatingWindowCheck);
    
    // Language selection
    QHBoxLayout *languageLayout = new QHBoxLayout();
    QLabel *languageLabel = new QLabel("Language:");
    languageLabel->setObjectName("settingsLabel");
    
    m_languageCombo = new QComboBox();
    m_languageCombo->setObjectName("settingsCombo");
    m_languageCombo->addItems({"English", "中文", "日本語", "한국어", "Français", "Deutsch", "Español"});
    m_languageCombo->setCurrentText("English");
    connect(m_languageCombo, QOverload<const QString &>::of(&QComboBox::currentTextChanged),
            this, &SettingsPage::onLanguageChanged);
    
    languageLayout->addWidget(languageLabel);
    languageLayout->addWidget(m_languageCombo);
    languageLayout->addStretch();
    
    generalLayout->addLayout(languageLayout);
    
    // Temperature unit selection
    QHBoxLayout *tempLayout = new QHBoxLayout();
    QLabel *tempLabel = new QLabel("Temperature display:");
    tempLabel->setObjectName("settingsLabel");
    
    m_temperatureCombo = new QComboBox();
    m_temperatureCombo->setObjectName("settingsCombo");
    m_temperatureCombo->addItems({"Celsius", "Fahrenheit"});
    m_temperatureCombo->setCurrentText("Celsius");
    connect(m_temperatureCombo, QOverload<const QString &>::of(&QComboBox::currentTextChanged),
            this, &SettingsPage::onTemperatureUnitChanged);
    
    tempLayout->addWidget(tempLabel);
    tempLayout->addWidget(m_temperatureCombo);
    tempLayout->addStretch();
    
    generalLayout->addLayout(tempLayout);
    
    m_leftLayout->addWidget(m_generalGroup);
    
    // Apply general settings styles
    setStyleSheet(R"(
        #settingsGroup {
            color: #ffffff;
            font-size: 14px;
            font-weight: bold;
            border: 1px solid #404040;
            border-radius: 8px;
            padding: 20px;
        }
        
        #settingsCheck {
            color: #cccccc;
            font-size: 12px;
        }
        
        #settingsLabel {
            color: #cccccc;
            font-size: 12px;
            min-width: 120px;
        }
        
        #settingsCombo {
            background-color: #404040;
            color: #ffffff;
            border: 1px solid #555555;
            border-radius: 4px;
            padding: 6px;
            min-width: 120px;
        }
    )");
}

void SettingsPage::setupServiceSettings()
{
    m_serviceGroup = new QGroupBox("Service & Software");
    m_serviceGroup->setObjectName("settingsGroup");
    
    QVBoxLayout *serviceLayout = new QVBoxLayout(m_serviceGroup);
    serviceLayout->setSpacing(15);
    
    // Service button
    m_serviceButton = new QPushButton("Service & Software");
    m_serviceButton->setObjectName("serviceButton");
    
    serviceLayout->addWidget(m_serviceButton);
    
    // Delay settings
    QHBoxLayout *delayLayout = new QHBoxLayout();
    
    QLabel *delayTextLabel = new QLabel("Delay the service startup by");
    delayTextLabel->setObjectName("settingsLabel");
    
    m_delaySpinBox = new QSpinBox();
    m_delaySpinBox->setObjectName("delaySpinBox");
    m_delaySpinBox->setRange(0, 60);
    m_delaySpinBox->setValue(3);
    m_delaySpinBox->setSuffix(" second(s)");
    
    m_applyDelayBtn = new QPushButton("Apply");
    m_applyDelayBtn->setObjectName("applyButton");
    
    connect(m_applyDelayBtn, &QPushButton::clicked, this, &SettingsPage::onApplyServiceDelay);
    
    delayLayout->addWidget(delayTextLabel);
    delayLayout->addWidget(m_delaySpinBox);
    delayLayout->addWidget(m_applyDelayBtn);
    delayLayout->addStretch();
    
    serviceLayout->addLayout(delayLayout);
    
    m_rightLayout->addWidget(m_serviceGroup);
    
    // Action buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    
    m_resetAllBtn = new QPushButton("Reset All");
    m_resetAllBtn->setObjectName("actionButton");
    
    m_applyBtn = new QPushButton("Apply");
    m_applyBtn->setObjectName("actionButton");
    
    connect(m_resetAllBtn, &QPushButton::clicked, this, &SettingsPage::onResetAll);
    
    buttonLayout->addWidget(m_resetAllBtn);
    buttonLayout->addWidget(m_applyBtn);
    buttonLayout->addStretch();
    
    m_rightLayout->addLayout(buttonLayout);
    m_rightLayout->addStretch();
    
    // Apply service settings styles
    setStyleSheet(R"(
        #serviceButton {
            background-color: #2a82da;
            color: #ffffff;
            border: none;
            padding: 10px 20px;
            border-radius: 4px;
            font-size: 14px;
            font-weight: bold;
        }
        
        #serviceButton:hover {
            background-color: #1e6bb8;
        }
        
        #delaySpinBox {
            background-color: #404040;
            color: #ffffff;
            border: 1px solid #555555;
            border-radius: 4px;
            padding: 6px;
            min-width: 100px;
        }
        
        #applyButton {
            background-color: #2a82da;
            color: #ffffff;
            border: none;
            padding: 6px 12px;
            border-radius: 4px;
            font-size: 12px;
        }
        
        #applyButton:hover {
            background-color: #1e6bb8;
        }
        
        #actionButton {
            background-color: #2a82da;
            color: #ffffff;
            border: none;
            padding: 10px 20px;
            border-radius: 4px;
            font-size: 14px;
            font-weight: bold;
            margin: 5px;
        }
        
        #actionButton:hover {
            background-color: #1e6bb8;
        }
    )");
}

void SettingsPage::applySettings()
{
    // Apply current settings
    onFloatingWindowToggled();
}

void SettingsPage::onAutoRunToggled()
{
    bool enabled = m_autoRunCheck->isChecked();
    // Handle auto-run setting
}

void SettingsPage::onHideOnTrayToggled()
{
    bool enabled = m_hideOnTrayCheck->isChecked();
    // Handle hide on tray setting
}

void SettingsPage::onMinimizeToTrayToggled()
{
    bool enabled = m_minimizeToTrayCheck->isChecked();
    // Handle minimize to tray setting
}

void SettingsPage::onFloatingWindowToggled()
{
    bool enabled = m_floatingWindowCheck->isChecked();
    m_toggleFrame->setStyleSheet(enabled ? 
        "background-color: #2a82da;" : 
        "background-color: #404040;");
}

void SettingsPage::onLanguageChanged()
{
    QString language = m_languageCombo->currentText();
    // Handle language change
}

void SettingsPage::onTemperatureUnitChanged()
{
    QString unit = m_temperatureCombo->currentText();
    // Handle temperature unit change
}

void SettingsPage::onServiceDelayChanged()
{
    int delay = m_delaySpinBox->value();
    // Handle service delay change
}

void SettingsPage::onApplyServiceDelay()
{
    int delay = m_delaySpinBox->value();
    // Apply service delay setting
}

void SettingsPage::onResetAll()
{
    // Reset all settings to default
    m_autoRunCheck->setChecked(true);
    m_hideOnTrayCheck->setChecked(true);
    m_minimizeToTrayCheck->setChecked(true);
    m_floatingWindowCheck->setChecked(true);
    m_languageCombo->setCurrentText("English");
    m_temperatureCombo->setCurrentText("Celsius");
    m_delaySpinBox->setValue(3);
    
    applySettings();
}
