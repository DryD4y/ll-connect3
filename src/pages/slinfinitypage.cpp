#include "slinfinitypage.h"
#include "widgets/fanwidget.h"
#include "lian_li_qt_integration.h"
#include <QTimer>
#include <QColorDialog>
#include <QMessageBox>

SLInfinityPage::SLInfinityPage(QWidget *parent)
    : QWidget(parent)
    , m_currentProfile("Quiet")
    , m_currentEffect("Rainbow")
    , m_currentSpeed(75)
    , m_currentBrightness(100)
    , m_directionLeft(false)
    , m_lianLi(nullptr)
    , m_colorButton(nullptr)
    , m_statusLabel(nullptr)
    , m_statusTimer(nullptr)
{
    // Initialize Lian Li integration
    m_lianLi = new LianLiQtIntegration(this);
    connect(m_lianLi, &LianLiQtIntegration::deviceConnected, this, &SLInfinityPage::onDeviceConnected);
    connect(m_lianLi, &LianLiQtIntegration::deviceDisconnected, this, &SLInfinityPage::onDeviceDisconnected);
    
    setupUI();
    setupFanVisualization();
    setupControls();
    updateFanVisualization();
    
    // Try to initialize the device
    if (m_lianLi->initialize()) {
        onDeviceConnected();
    } else {
        onDeviceDisconnected();
    }
}

void SLInfinityPage::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(20, 20, 20, 20);
    m_mainLayout->setSpacing(20);
    
    // Header
    m_headerLayout = new QHBoxLayout();
    m_headerLayout->setContentsMargins(0, 0, 0, 0);
    
    m_controllerLabel = new QLabel("SL-Inf Controller01");
    m_controllerLabel->setObjectName("controllerLabel");
    
    m_exportBtn = new QPushButton("↑ Export");
    m_exportBtn->setObjectName("actionButton");
    
    m_importBtn = new QPushButton("↓ Import");
    m_importBtn->setObjectName("actionButton");
    
    m_headerLayout->addWidget(m_controllerLabel);
    m_headerLayout->addStretch();
    m_headerLayout->addWidget(m_exportBtn);
    m_headerLayout->addWidget(m_importBtn);
    
    connect(m_exportBtn, &QPushButton::clicked, this, &SLInfinityPage::onExport);
    connect(m_importBtn, &QPushButton::clicked, this, &SLInfinityPage::onImport);
    
    m_mainLayout->addLayout(m_headerLayout);
    
    // Content layout
    m_contentLayout = new QHBoxLayout();
    m_contentLayout->setSpacing(20);
    
    m_leftLayout = new QVBoxLayout();
    m_rightLayout = new QVBoxLayout();
    
    m_contentLayout->addLayout(m_leftLayout, 2);
    m_contentLayout->addLayout(m_rightLayout, 1);
    
    m_mainLayout->addLayout(m_contentLayout);
    
    // Apply styles
    setStyleSheet(R"(
        #controllerLabel {
            color: #ffffff;
            font-size: 18px;
            font-weight: bold;
        }
        
        #actionButton {
            background-color: #2a82da;
            color: #ffffff;
            border: none;
            padding: 8px 16px;
            border-radius: 4px;
            margin-left: 8px;
        }
        
        #actionButton:hover {
            background-color: #1e6bb8;
        }
    )");
}

void SLInfinityPage::setupFanVisualization()
{
    m_fanGroup = new QGroupBox("Fan Configuration");
    m_fanGroup->setObjectName("fanGroup");
    
    m_fanGrid = new QGridLayout(m_fanGroup);
    m_fanGrid->setSpacing(10);
    
    // Create fan widgets for 4 ports, 4 fans each
    for (int port = 0; port < 4; ++port) {
        for (int fan = 0; fan < 4; ++fan) {
            m_fanWidgets[port][fan] = new FanWidget(port, fan);
            m_fanGrid->addWidget(m_fanWidgets[port][fan], port, fan);
            
            connect(m_fanWidgets[port][fan], &FanWidget::clicked, [this, port, fan]() {
                // Handle fan click
            });
        }
    }
    
    m_leftLayout->addWidget(m_fanGroup);
    
    // Apply fan group styles
    setStyleSheet(R"(
        #fanGroup {
            color: #ffffff;
            font-size: 14px;
            font-weight: bold;
            border: 1px solid #404040;
            border-radius: 8px;
            padding: 15px;
        }
    )");
}

void SLInfinityPage::setupControls()
{
    // Fan Profile controls
    m_profileGroup = new QGroupBox("Fan Profile");
    m_profileGroup->setObjectName("controlGroup");
    
    QVBoxLayout *profileLayout = new QVBoxLayout(m_profileGroup);
    
    m_profileCombo = new QComboBox();
    m_profileCombo->setObjectName("profileCombo");
    m_profileCombo->addItems({"Quiet", "StdSP", "HighSP", "FullSP", "Custom"});
    m_profileCombo->setCurrentText("Quiet");
    
    m_applyToAllBtn = new QPushButton("Apply To All");
    m_applyToAllBtn->setObjectName("actionButton");
    
    connect(m_profileCombo, QOverload<const QString &>::of(&QComboBox::currentTextChanged),
            this, &SLInfinityPage::onProfileChanged);
    connect(m_applyToAllBtn, &QPushButton::clicked, this, &SLInfinityPage::onApplyToAll);
    
    profileLayout->addWidget(m_profileCombo);
    profileLayout->addWidget(m_applyToAllBtn);
    
    m_rightLayout->addWidget(m_profileGroup);
    
    // Lighting Effects controls
    m_lightingGroup = new QGroupBox("Lighting Effects");
    m_lightingGroup->setObjectName("controlGroup");
    
    QVBoxLayout *lightingLayout = new QVBoxLayout(m_lightingGroup);
    lightingLayout->setSpacing(10);
    
    // Effect selection
    m_effectCombo = new QComboBox();
    m_effectCombo->setObjectName("effectCombo");
    m_effectCombo->addItems({
        "Rainbow", "Rainbow Morph", "Static Color", "Breathing", 
        "Breathing Rainbow", "Runway", "Mop up", "Meteor", "Warning",
        "Voice", "Mixing", "Stack", "Tide", "Scan", "Door",
        "Heart Beat", "Heart Beat Runway", "Disco", "Electric Current"
    });
    m_effectCombo->setCurrentText("Rainbow");
    
    connect(m_effectCombo, QOverload<const QString &>::of(&QComboBox::currentTextChanged),
            this, &SLInfinityPage::onEffectChanged);
    
    lightingLayout->addWidget(m_effectCombo);
    
    // Speed slider
    QHBoxLayout *speedLayout = new QHBoxLayout();
    QLabel *speedLabel = new QLabel("SPEED");
    speedLabel->setObjectName("sliderLabel");
    
    m_speedSlider = new QSlider(Qt::Horizontal);
    m_speedSlider->setObjectName("customSlider");
    m_speedSlider->setRange(0, 100);
    m_speedSlider->setValue(75);
    
    QLabel *speedValueLabel = new QLabel("75%");
    speedValueLabel->setObjectName("sliderValue");
    
    connect(m_speedSlider, &QSlider::valueChanged, [this, speedValueLabel](int value) {
        speedValueLabel->setText(QString::number(value) + "%");
        onSpeedChanged(value);
    });
    
    speedLayout->addWidget(speedLabel);
    speedLayout->addWidget(m_speedSlider);
    speedLayout->addWidget(speedValueLabel);
    
    lightingLayout->addLayout(speedLayout);
    
    // Brightness slider
    QHBoxLayout *brightnessLayout = new QHBoxLayout();
    QLabel *brightnessLabel = new QLabel("BRIGHTNESS");
    brightnessLabel->setObjectName("sliderLabel");
    
    m_brightnessSlider = new QSlider(Qt::Horizontal);
    m_brightnessSlider->setObjectName("customSlider");
    m_brightnessSlider->setRange(0, 100);
    m_brightnessSlider->setValue(100);
    
    QLabel *brightnessValueLabel = new QLabel("100%");
    brightnessValueLabel->setObjectName("sliderValue");
    
    connect(m_brightnessSlider, &QSlider::valueChanged, [this, brightnessValueLabel](int value) {
        brightnessValueLabel->setText(QString::number(value) + "%");
        onBrightnessChanged(value);
    });
    
    brightnessLayout->addWidget(brightnessLabel);
    brightnessLayout->addWidget(m_brightnessSlider);
    brightnessLayout->addWidget(brightnessValueLabel);
    
    lightingLayout->addLayout(brightnessLayout);
    
    // Direction controls
    QLabel *directionLabel = new QLabel("DIRECTION");
    directionLabel->setObjectName("sliderLabel");
    lightingLayout->addWidget(directionLabel);
    
    m_directionLayout = new QHBoxLayout();
    m_directionLayout->setSpacing(10);
    
    m_leftDirectionBtn = new QPushButton("<<<<");
    m_leftDirectionBtn->setObjectName("directionButton");
    m_leftDirectionBtn->setCheckable(true);
    
    m_rightDirectionBtn = new QPushButton(">>>>");
    m_rightDirectionBtn->setObjectName("directionButton");
    m_rightDirectionBtn->setCheckable(true);
    m_rightDirectionBtn->setChecked(true);
    
    connect(m_leftDirectionBtn, &QPushButton::clicked, this, &SLInfinityPage::onDirectionChanged);
    connect(m_rightDirectionBtn, &QPushButton::clicked, this, &SLInfinityPage::onDirectionChanged);
    
    m_directionLayout->addWidget(m_leftDirectionBtn);
    m_directionLayout->addWidget(m_rightDirectionBtn);
    m_directionLayout->addStretch();
    
    lightingLayout->addLayout(m_directionLayout);
    
    // Color picker button
    QLabel *colorLabel = new QLabel("COLOR");
    colorLabel->setObjectName("sliderLabel");
    lightingLayout->addWidget(colorLabel);
    
    m_colorButton = new QPushButton("Choose Color");
    m_colorButton->setObjectName("colorButton");
    m_colorButton->setStyleSheet("background-color: #ff0000; border: 2px solid #333; color: white; font-weight: bold;");
    m_colorButton->setMinimumHeight(40);
    
    connect(m_colorButton, &QPushButton::clicked, this, &SLInfinityPage::onColorButtonClicked);
    
    lightingLayout->addWidget(m_colorButton);
    
    // Status label
    m_statusLabel = new QLabel("");
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setMinimumHeight(20);
    lightingLayout->addWidget(m_statusLabel);
    
    // Action buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    
    m_applyToAllLightingBtn = new QPushButton("Apply To All");
    m_applyToAllLightingBtn->setObjectName("actionButton");
    
    m_defaultBtn = new QPushButton("Default");
    m_defaultBtn->setObjectName("actionButton");
    
    connect(m_applyToAllLightingBtn, &QPushButton::clicked, this, &SLInfinityPage::onApplyToAll);
    connect(m_defaultBtn, &QPushButton::clicked, this, &SLInfinityPage::onDefault);
    
    buttonLayout->addWidget(m_applyToAllLightingBtn);
    buttonLayout->addWidget(m_defaultBtn);
    
    lightingLayout->addLayout(buttonLayout);
    lightingLayout->addStretch();
    
    m_rightLayout->addWidget(m_lightingGroup);
    
    // Apply control styles
    setStyleSheet(R"(
        #controlGroup {
            color: #ffffff;
            font-size: 14px;
            font-weight: bold;
            border: 1px solid #404040;
            border-radius: 8px;
            padding: 15px;
            margin-top: 10px;
        }
        
        #profileCombo, #effectCombo {
            background-color: #404040;
            color: #ffffff;
            border: 1px solid #555555;
            border-radius: 4px;
            padding: 8px;
            font-size: 12px;
        }
        
        #sliderLabel {
            color: #cccccc;
            font-size: 12px;
            font-weight: bold;
            min-width: 80px;
        }
        
        #customSlider {
            background-color: transparent;
        }
        
        #customSlider::groove:horizontal {
            background-color: #404040;
            height: 6px;
            border-radius: 3px;
        }
        
        #customSlider::handle:horizontal {
            background-color: #2a82da;
            width: 16px;
            height: 16px;
            border-radius: 8px;
            margin: -5px 0;
        }
        
        #customSlider::sub-page:horizontal {
            background-color: #2a82da;
            border-radius: 3px;
        }
        
        #sliderValue {
            color: #ffffff;
            font-size: 12px;
            font-weight: bold;
            min-width: 40px;
        }
        
        #directionButton {
            background-color: #404040;
            color: #cccccc;
            border: 1px solid #555555;
            border-radius: 4px;
            padding: 8px 16px;
            font-size: 12px;
            font-weight: bold;
        }
        
        #directionButton:checked {
            background-color: #2a82da;
            color: #ffffff;
        }
    )");
}

void SLInfinityPage::updateFanVisualization()
{
    // Update fan widgets with current settings
    for (int port = 0; port < 4; ++port) {
        for (int fan = 0; fan < 4; ++fan) {
            FanWidget *fanWidget = m_fanWidgets[port][fan];
            
            // Set RPM based on profile
            int rpm = 0;
            if (m_currentProfile == "Quiet") rpm = 950 + (port * 10) + (fan * 5);
            else if (m_currentProfile == "StdSP") rpm = 1100 + (port * 15) + (fan * 8);
            else if (m_currentProfile == "HighSP") rpm = 1300 + (port * 20) + (fan * 10);
            else if (m_currentProfile == "FullSP") rpm = 1500 + (port * 25) + (fan * 12);
            
            if (port == 2) rpm = 0; // Port 3 is off
            
            fanWidget->setRPM(rpm);
            fanWidget->setProfile(m_currentProfile);
            fanWidget->setLighting(m_currentEffect);
            fanWidget->setActive(rpm > 0);
        }
    }
}

void SLInfinityPage::onProfileChanged()
{
    m_currentProfile = m_profileCombo->currentText();
    updateFanVisualization();
}

void SLInfinityPage::onEffectChanged()
{
    m_currentEffect = m_effectCombo->currentText();
    updateFanVisualization();
    
    // Apply effect to device if connected
    if (m_lianLi && m_lianLi->isConnected()) {
        if (m_currentEffect == "Rainbow") {
            m_lianLi->setRainbowEffect();
        } else if (m_currentEffect == "Breathing") {
            QColor currentColor = m_colorButton->palette().button().color();
            m_lianLi->setBreathingEffect(currentColor);
        } else if (m_currentEffect == "Static Color") {
            QColor currentColor = m_colorButton->palette().button().color();
            m_lianLi->setAllChannelsColor(currentColor);
        }
    }
}

void SLInfinityPage::onSpeedChanged(int value)
{
    m_currentSpeed = value;
    updateFanVisualization();
}

void SLInfinityPage::onBrightnessChanged(int value)
{
    m_currentBrightness = value;
    updateFanVisualization();
}

void SLInfinityPage::onDirectionChanged()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (!button) return;
    
    if (button == m_leftDirectionBtn) {
        m_leftDirectionBtn->setChecked(true);
        m_rightDirectionBtn->setChecked(false);
        m_directionLeft = true;
    } else {
        m_leftDirectionBtn->setChecked(false);
        m_rightDirectionBtn->setChecked(true);
        m_directionLeft = false;
    }
    
    updateFanVisualization();
}

void SLInfinityPage::onApplyToAll()
{
    // Apply current settings to all fans
    updateFanVisualization();
}

void SLInfinityPage::onDefault()
{
    m_profileCombo->setCurrentText("Quiet");
    m_effectCombo->setCurrentText("Rainbow");
    m_speedSlider->setValue(75);
    m_brightnessSlider->setValue(100);
    m_rightDirectionBtn->setChecked(true);
    m_leftDirectionBtn->setChecked(false);
    
    m_currentProfile = "Quiet";
    m_currentEffect = "Rainbow";
    m_currentSpeed = 75;
    m_currentBrightness = 100;
    m_directionLeft = false;
    
    updateFanVisualization();
}

void SLInfinityPage::onExport()
{
    // Handle export functionality
}

void SLInfinityPage::onImport()
{
    // Handle import functionality
}

void SLInfinityPage::onColorButtonClicked()
{
    if (!m_lianLi || !m_lianLi->isConnected()) {
        QMessageBox::warning(this, "Device Not Connected", 
            "Lian Li device is not connected. Please check your device and try again.");
        return;
    }
    
    QColor currentColor = m_colorButton->palette().button().color();
    QColor color = QColorDialog::getColor(currentColor, this, "Choose LED Color");
    
    if (color.isValid()) {
        // Update button color
        m_colorButton->setStyleSheet(QString("background-color: %1; border: 2px solid #333;").arg(color.name()));
        
        // Apply color to all channels
        if (m_lianLi->setAllChannelsColor(color)) {
            m_statusLabel->setText("✓ Color applied successfully!");
            m_statusLabel->setStyleSheet("color: green; font-weight: bold;");
        } else {
            m_statusLabel->setText("✗ Failed to apply color");
            m_statusLabel->setStyleSheet("color: red; font-weight: bold;");
        }
        
        // Clear status after 3 seconds
        if (m_statusTimer) {
            m_statusTimer->stop();
        }
        m_statusTimer = new QTimer(this);
        m_statusTimer->setSingleShot(true);
        m_statusTimer->setInterval(3000);
        connect(m_statusTimer, &QTimer::timeout, [this]() {
            m_statusLabel->setText("");
            m_statusLabel->setStyleSheet("");
        });
        m_statusTimer->start();
    }
}

void SLInfinityPage::onDeviceConnected()
{
    if (m_statusLabel) {
        m_statusLabel->setText("✓ Lian Li SL Infinity connected");
        m_statusLabel->setStyleSheet("color: green; font-weight: bold;");
    }
    
    // Enable controls
    if (m_colorButton) {
        m_colorButton->setEnabled(true);
    }
}

void SLInfinityPage::onDeviceDisconnected()
{
    if (m_statusLabel) {
        m_statusLabel->setText("✗ Lian Li SL Infinity disconnected");
        m_statusLabel->setStyleSheet("color: red; font-weight: bold;");
    }
    
    // Disable controls
    if (m_colorButton) {
        m_colorButton->setEnabled(false);
    }
}
