#include "lightingpage.h"
#include "widgets/customslider.h"
#include "widgets/fanlightingwidget.h"
#include "lian_li_qt_integration.h"
#include <QFont>
#include <QDebug>
#include <QColorDialog>

LightingPage::LightingPage(QWidget *parent)
    : QWidget(parent)
    , m_currentEffect("Rainbow")
    , m_currentSpeed(50)
    , m_currentBrightness(100)
    , m_directionLeft(false)
    , m_lianLi(nullptr)
{
    // Initialize port colors to white
    m_portColors[0] = QColor(255, 255, 255); // Port 1 - White
    m_portColors[1] = QColor(255, 255, 255); // Port 2 - White
    m_portColors[2] = QColor(255, 255, 255); // Port 3 - White
    m_portColors[3] = QColor(255, 255, 255); // Port 4 - White
    
    // Initialize Lian Li integration
    m_lianLi = new LianLiQtIntegration(this);
    connect(m_lianLi, &LianLiQtIntegration::deviceConnected, this, &LightingPage::onDeviceConnected);
    connect(m_lianLi, &LianLiQtIntegration::deviceDisconnected, this, &LightingPage::onDeviceDisconnected);
    
    setupUI();
    setupControls();
    setupProductDemo();
    updateLightingPreview();
    
    // Try to initialize the device
    if (m_lianLi->initialize()) {
        onDeviceConnected();
    } else {
        qDebug() << "Lian Li device not connected";
    }
}

void LightingPage::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(20, 20, 20, 20);
    m_mainLayout->setSpacing(20);
    
    // Content layout
    m_contentLayout = new QHBoxLayout();
    m_contentLayout->setSpacing(30);
    
    m_leftLayout = new QVBoxLayout();
    m_rightLayout = new QVBoxLayout();
    
    m_contentLayout->addLayout(m_leftLayout, 1);
    m_contentLayout->addLayout(m_rightLayout, 1);  // Changed from 2 to 1 for equal sizing
    
    m_mainLayout->addLayout(m_contentLayout);
}

void LightingPage::setupControls()
{
    // Lighting Effects group
    m_lightingGroup = new QGroupBox("Lighting Effects");
    m_lightingGroup->setObjectName("controlGroup");
    
    QVBoxLayout *lightingLayout = new QVBoxLayout(m_lightingGroup);
    lightingLayout->setSpacing(15);
    
    // Effect selection
    QLabel *effectLabel = new QLabel("Lighting Effects");
    effectLabel->setObjectName("controlLabel");
    
    m_effectCombo = new QComboBox();
    m_effectCombo->setObjectName("effectCombo");
    m_effectCombo->addItems({
        "Rainbow",
        "Rainbow Morph",
        "Static Color",
        "Breathing",
        "Meteor",
        "Runway"
    });
    m_effectCombo->setCurrentText("Rainbow");
    
    connect(m_effectCombo, QOverload<const QString &>::of(&QComboBox::currentTextChanged),
            this, &LightingPage::onEffectChanged);
    
    lightingLayout->addWidget(effectLabel);
    lightingLayout->addWidget(m_effectCombo);
    
    // Static Color specific controls
    m_staticColorWidget = new QWidget();
    QVBoxLayout *staticColorLayout = new QVBoxLayout(m_staticColorWidget);
    staticColorLayout->setContentsMargins(0, 0, 0, 0);
    
    QLabel *colorLabel = new QLabel("PORT COLORS");
    colorLabel->setObjectName("controlLabel");
    staticColorLayout->addWidget(colorLabel);
    
    m_colorBoxLayout = new QHBoxLayout();
    m_colorBoxLayout->setSpacing(10);
    
    for (int i = 0; i < 4; ++i) {
        m_colorButtons[i] = new QPushButton();
        m_colorButtons[i]->setObjectName("colorButton");
        m_colorButtons[i]->setFixedSize(40, 40);
        m_colorButtons[i]->setProperty("portIndex", i);
        updateColorButton(i);
        
        connect(m_colorButtons[i], &QPushButton::clicked, this, &LightingPage::onColorButtonClicked);
        m_colorBoxLayout->addWidget(m_colorButtons[i]);
    }
    m_colorBoxLayout->addStretch();
    staticColorLayout->addLayout(m_colorBoxLayout);
    
    lightingLayout->addWidget(m_staticColorWidget);
    
    // Initially hide static color widget (only show for Static Color effect)
    m_staticColorWidget->setVisible(false);
    
    // Speed slider (25% increments: 0, 25, 50, 75, 100)
    m_speedSlider = new CustomSlider("SPEED");
    m_speedSlider->setSnapToIncrements(true, 25);  // Enable 25% snapping first
    m_speedSlider->setRange(0, 100);  // This will be converted to 0-4 internally
    m_speedSlider->setValue(50);  // Default to 50% (medium speed)
    connect(m_speedSlider, &CustomSlider::valueChanged, this, &LightingPage::onSpeedChanged);
    lightingLayout->addWidget(m_speedSlider);
    
    // Brightness slider (25% increments: 0, 25, 50, 75, 100)
    m_brightnessSlider = new CustomSlider("BRIGHTNESS");
    m_brightnessSlider->setSnapToIncrements(true, 25);  // Enable 25% snapping first
    m_brightnessSlider->setRange(0, 100);  // This will be converted to 0-4 internally
    m_brightnessSlider->setValue(100);  // Default to 100% (full brightness)
    connect(m_brightnessSlider, &CustomSlider::valueChanged, this, &LightingPage::onBrightnessChanged);
    lightingLayout->addWidget(m_brightnessSlider);
    
    // Direction controls (wrapped in a widget for show/hide)
    m_directionWidget = new QWidget();
    QVBoxLayout *directionWidgetLayout = new QVBoxLayout(m_directionWidget);
    directionWidgetLayout->setContentsMargins(0, 0, 0, 0);
    
    QLabel *directionLabel = new QLabel("DIRECTION");
    directionLabel->setObjectName("controlLabel");
    directionWidgetLayout->addWidget(directionLabel);
    
    m_directionLayout = new QHBoxLayout();
    m_directionLayout->setSpacing(10);
    
    m_leftDirectionBtn = new QPushButton("<<<<");
    m_leftDirectionBtn->setObjectName("directionButton");
    m_leftDirectionBtn->setCheckable(true);
    
    m_rightDirectionBtn = new QPushButton(">>>>");
    m_rightDirectionBtn->setObjectName("directionButton");
    m_rightDirectionBtn->setCheckable(true);
    m_rightDirectionBtn->setChecked(true);
    
    connect(m_leftDirectionBtn, &QPushButton::clicked, this, &LightingPage::onDirectionChanged);
    connect(m_rightDirectionBtn, &QPushButton::clicked, this, &LightingPage::onDirectionChanged);
    
    m_directionLayout->addWidget(m_leftDirectionBtn);
    m_directionLayout->addWidget(m_rightDirectionBtn);
    m_directionLayout->addStretch();
    
    directionWidgetLayout->addLayout(m_directionLayout);
    lightingLayout->addWidget(m_directionWidget);
    
    // Apply button
    m_applyBtn = new QPushButton("Apply");
    m_applyBtn->setObjectName("applyButton");
    connect(m_applyBtn, &QPushButton::clicked, this, &LightingPage::onApply);
    
    lightingLayout->addWidget(m_applyBtn);
    lightingLayout->addStretch();
    
    m_leftLayout->addWidget(m_lightingGroup);
    
    // Apply control styles
    setStyleSheet(R"(
        #controlGroup {
            color: #ffffff;
            font-size: 14px;
            font-weight: bold;
            border: 1px solid #404040;
            border-radius: 8px;
            padding: 15px;
        }
        
        #controlLabel {
            color: #cccccc;
            font-size: 12px;
            font-weight: bold;
        }
        
        #effectCombo {
            background-color: #404040;
            color: #ffffff;
            border: 1px solid #555555;
            border-radius: 4px;
            padding: 8px;
            font-size: 12px;
        }
        
        #effectCombo::drop-down {
            border: none;
        }
        
        #effectCombo::down-arrow {
            image: none;
            border-left: 5px solid transparent;
            border-right: 5px solid transparent;
            border-top: 5px solid #cccccc;
            margin-right: 8px;
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
        
        #applyButton {
            background-color: #2a82da;
            color: #ffffff;
            border: none;
            padding: 10px 20px;
            border-radius: 4px;
            font-size: 14px;
            font-weight: bold;
        }
        
        #applyButton:hover {
            background-color: #1e6bb8;
        }
        
        #colorButton {
            border: 2px solid #555555;
            border-radius: 4px;
            min-width: 40px;
            min-height: 40px;
        }
        
        #colorButton:hover {
            border-color: #2a82da;
        }
    )");
}

void LightingPage::setupProductDemo()
{
    // Product demo header
    m_demoLabel = new QLabel("Product Demo");
    m_demoLabel->setObjectName("demoLabel");
    
    m_rightLayout->addWidget(m_demoLabel);
    
    // Fan lighting visualization
    m_fanLightingWidget = new FanLightingWidget();
    m_fanLightingWidget->setMinimumSize(350, 250);  // Smaller minimum size
    m_fanLightingWidget->setMaximumHeight(300);     // Limit maximum height
    m_fanLightingWidget->setObjectName("fanLightingWidget");
    
    m_rightLayout->addWidget(m_fanLightingWidget);
    m_rightLayout->addStretch();
    
    // Apply demo styles
    setStyleSheet(R"(
        #demoLabel {
            color: #ffffff;
            font-size: 16px;
            font-weight: bold;
        }
        
        #fanLightingWidget {
            background-color: #1a1a1a;
            border: 2px solid #404040;
            border-radius: 8px;
        }
    )");
}

void LightingPage::updateLightingPreview()
{
    // Update the fan lighting widget with current settings
    if (m_fanLightingWidget) {
        m_fanLightingWidget->setEffect(m_currentEffect);
        m_fanLightingWidget->setSpeed(m_currentSpeed);
        m_fanLightingWidget->setBrightness(m_currentBrightness);
        m_fanLightingWidget->setDirection(m_directionLeft);
        
        // Set color for effects that use it
        if (m_currentEffect == "Static Color") {
            // For static color, pass the port colors
            m_fanLightingWidget->setPortColors(m_portColors);
        } else if (m_currentEffect == "Breathing") {
            m_fanLightingWidget->setColor(QColor(255, 255, 255)); // White for breathing
        } else if (m_currentEffect == "Meteor") {
            m_fanLightingWidget->setColor(QColor(100, 200, 255)); // Blue-white for meteor
        } else if (m_currentEffect == "Runway") {
            m_fanLightingWidget->setColor(QColor(255, 200, 100)); // Orange for runway
        } else {
            m_fanLightingWidget->setColor(QColor(255, 255, 255)); // Default color
        }
    }
}

void LightingPage::onEffectChanged()
{
    m_currentEffect = m_effectCombo->currentText();
    
    // Show/hide controls based on effect
    bool isStaticColor = (m_currentEffect == "Static Color");
    m_staticColorWidget->setVisible(isStaticColor);
    m_speedSlider->setVisible(!isStaticColor);
    m_directionWidget->setVisible(!isStaticColor);
    
    updateLightingPreview();
}

void LightingPage::onSpeedChanged(int value)
{
    m_currentSpeed = value;
    updateLightingPreview();
}

void LightingPage::onBrightnessChanged(int value)
{
    m_currentBrightness = value;
    updateLightingPreview();
}

void LightingPage::onDirectionChanged()
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
    
    updateLightingPreview();
}

void LightingPage::onApply()
{
    // Update preview
    updateLightingPreview();
    
    // Apply lighting settings to device if connected
    if (!m_lianLi || !m_lianLi->isConnected()) {
        qDebug() << "Device not connected - cannot apply lighting";
        return;
    }
    
    bool success = false;
    
    qDebug() << "Applying effect:" << m_currentEffect 
             << "Speed:" << m_currentSpeed 
             << "Brightness:" << m_currentBrightness 
             << "Direction:" << (m_directionLeft ? "Left" : "Right");
    
    if (m_currentEffect == "Rainbow") {
        success = m_lianLi->setRainbowEffect(m_currentSpeed, m_currentBrightness, m_directionLeft);
    } else if (m_currentEffect == "Rainbow Morph") {
        success = m_lianLi->setRainbowMorphEffect(m_currentSpeed, m_currentBrightness, m_directionLeft);
    } else if (m_currentEffect == "Static Color") {
        // For static color, set each port to its individual color with brightness
        // Map ports to channels: Port 1->Channel 0, Port 2->Channel 1, Port 3->Channel 2, Port 4->Channel 3
        success = true;
        for (int port = 0; port < 4; ++port) {
            qDebug() << "Setting Port" << (port + 1) << "to color" << m_portColors[port] << "on channel" << port << "with brightness" << m_currentBrightness;
            if (!m_lianLi->setChannelColor(port, m_portColors[port], m_currentBrightness)) {
                qDebug() << "Failed to set Port" << (port + 1) << "on channel" << port;
                success = false;
            } else {
                qDebug() << "Successfully set Port" << (port + 1) << "on channel" << port;
            }
        }
    } else if (m_currentEffect == "Breathing") {
        // For breathing, use white as default (could add color picker later)
        success = m_lianLi->setBreathingEffect(QColor(255, 255, 255), m_currentSpeed, m_currentBrightness, m_directionLeft);
    } else if (m_currentEffect == "Meteor") {
        success = m_lianLi->setMeteorEffect(m_currentSpeed, m_currentBrightness, m_directionLeft);
    } else if (m_currentEffect == "Runway") {
        success = m_lianLi->setRunwayEffect(m_currentSpeed, m_currentBrightness, m_directionLeft);
    }
    
    if (success) {
        qDebug() << "✓ Successfully applied effect:" << m_currentEffect;
    } else {
        qDebug() << "✗ Failed to apply effect:" << m_currentEffect;
    }
}

void LightingPage::onDeviceConnected()
{
    qDebug() << "Lian Li device connected";
}

void LightingPage::onDeviceDisconnected()
{
    qDebug() << "Lian Li device disconnected";
}

void LightingPage::onColorButtonClicked()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (!button) return;
    
    int portIndex = button->property("portIndex").toInt();
    if (portIndex < 0 || portIndex >= 4) return;
    
    QColor currentColor = m_portColors[portIndex];
    QColor newColor = QColorDialog::getColor(currentColor, this, 
        QString("Select Color for Port %1").arg(portIndex + 1));
    
    if (newColor.isValid()) {
        m_portColors[portIndex] = newColor;
        updateColorButton(portIndex);
        updateLightingPreview();
    }
}

void LightingPage::updateColorButton(int portIndex)
{
    if (portIndex < 0 || portIndex >= 4) return;
    
    QColor color = m_portColors[portIndex];
    QString style = QString("QPushButton { background-color: %1; border: 2px solid #555555; border-radius: 4px; }")
                   .arg(color.name());
    m_colorButtons[portIndex]->setStyleSheet(style);
}
