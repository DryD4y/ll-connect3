#include "lightingpage.h"
#include "widgets/customslider.h"
#include <QFont>

LightingPage::LightingPage(QWidget *parent)
    : QWidget(parent)
    , m_currentEffect("Rainbow")
    , m_currentSpeed(75)
    , m_currentBrightness(100)
    , m_directionLeft(false)
{
    setupUI();
    setupControls();
    setupProductDemo();
    updateLightingPreview();
}

void LightingPage::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(20, 20, 20, 20);
    m_mainLayout->setSpacing(20);
    
    // Header
    m_headerLayout = new QHBoxLayout();
    m_headerLayout->setContentsMargins(0, 0, 0, 0);
    
    m_mbLightingSyncCheck = new QCheckBox("MB Lighting Sync");
    m_mbLightingSyncCheck->setObjectName("headerCheck");
    
    m_exportBtn = new QPushButton("↑ Export");
    m_exportBtn->setObjectName("actionButton");
    
    m_importBtn = new QPushButton("↓ Import");
    m_importBtn->setObjectName("actionButton");
    
    m_headerLayout->addWidget(m_mbLightingSyncCheck);
    m_headerLayout->addStretch();
    m_headerLayout->addWidget(m_exportBtn);
    m_headerLayout->addWidget(m_importBtn);
    
    m_mainLayout->addLayout(m_headerLayout);
    
    // Content layout
    m_contentLayout = new QHBoxLayout();
    m_contentLayout->setSpacing(30);
    
    m_leftLayout = new QVBoxLayout();
    m_rightLayout = new QVBoxLayout();
    
    m_contentLayout->addLayout(m_leftLayout, 1);
    m_contentLayout->addLayout(m_rightLayout, 2);
    
    m_mainLayout->addLayout(m_contentLayout);
    
    // Apply styles
    setStyleSheet(R"(
        #headerCheck {
            color: #ffffff;
            font-size: 14px;
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
        "Rainbow", "Rainbow Morph", "Static Color", "Breathing", 
        "Breathing Rainbow", "Runway", "Mop up", "Meteor", "Warning",
        "Voice", "Mixing", "Stack", "Tide", "Scan", "Door",
        "Heart Beat", "Heart Beat Runway", "Disco", "Electric Current"
    });
    m_effectCombo->setCurrentText("Rainbow");
    
    connect(m_effectCombo, QOverload<const QString &>::of(&QComboBox::currentTextChanged),
            this, &LightingPage::onEffectChanged);
    
    lightingLayout->addWidget(effectLabel);
    lightingLayout->addWidget(m_effectCombo);
    
    // Speed slider
    m_speedSlider = new CustomSlider("SPEED");
    m_speedSlider->setValue(75);
    connect(m_speedSlider, &CustomSlider::valueChanged, this, &LightingPage::onSpeedChanged);
    lightingLayout->addWidget(m_speedSlider);
    
    // Brightness slider
    m_brightnessSlider = new CustomSlider("BRIGHTNESS");
    m_brightnessSlider->setValue(100);
    connect(m_brightnessSlider, &CustomSlider::valueChanged, this, &LightingPage::onBrightnessChanged);
    lightingLayout->addWidget(m_brightnessSlider);
    
    // Direction controls
    QLabel *directionLabel = new QLabel("DIRECTION");
    directionLabel->setObjectName("controlLabel");
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
    
    connect(m_leftDirectionBtn, &QPushButton::clicked, this, &LightingPage::onDirectionChanged);
    connect(m_rightDirectionBtn, &QPushButton::clicked, this, &LightingPage::onDirectionChanged);
    
    m_directionLayout->addWidget(m_leftDirectionBtn);
    m_directionLayout->addWidget(m_rightDirectionBtn);
    m_directionLayout->addStretch();
    
    lightingLayout->addLayout(m_directionLayout);
    
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
    )");
}

void LightingPage::setupProductDemo()
{
    // Product demo header
    QHBoxLayout *demoHeaderLayout = new QHBoxLayout();
    
    m_demoLabel = new QLabel("Product Demo");
    m_demoLabel->setObjectName("demoLabel");
    
    m_productCombo = new QComboBox();
    m_productCombo->setObjectName("productCombo");
    m_productCombo->addItems({"SL Fan", "SL Infinity", "UNI HUB"});
    m_productCombo->setCurrentText("SL Fan");
    
    demoHeaderLayout->addWidget(m_demoLabel);
    demoHeaderLayout->addStretch();
    demoHeaderLayout->addWidget(m_productCombo);
    
    m_rightLayout->addLayout(demoHeaderLayout);
    
    // Demo image placeholder
    m_demoImageLabel = new QLabel();
    m_demoImageLabel->setObjectName("demoImage");
    m_demoImageLabel->setMinimumSize(400, 300);
    m_demoImageLabel->setAlignment(Qt::AlignCenter);
    m_demoImageLabel->setText("PC Build Demo\n(RGB Lighting Preview)");
    
    m_rightLayout->addWidget(m_demoImageLabel);
    m_rightLayout->addStretch();
    
    // Apply demo styles
    setStyleSheet(R"(
        #demoLabel {
            color: #ffffff;
            font-size: 16px;
            font-weight: bold;
        }
        
        #productCombo {
            background-color: #404040;
            color: #ffffff;
            border: 1px solid #555555;
            border-radius: 4px;
            padding: 6px;
        }
        
        #demoImage {
            background-color: #1a1a1a;
            border: 2px solid #404040;
            border-radius: 8px;
            color: #888888;
            font-size: 14px;
        }
    )");
}

void LightingPage::updateLightingPreview()
{
    // Update the demo image based on current settings
    QString previewText = QString("PC Build Demo\n\nEffect: %1\nSpeed: %2%\nBrightness: %3%\nDirection: %4")
                         .arg(m_currentEffect)
                         .arg(m_currentSpeed)
                         .arg(m_currentBrightness)
                         .arg(m_directionLeft ? "Left" : "Right");
    
    m_demoImageLabel->setText(previewText);
}

void LightingPage::onEffectChanged()
{
    m_currentEffect = m_effectCombo->currentText();
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
    // Apply lighting settings
    updateLightingPreview();
}

void LightingPage::onMbLightingSyncToggled()
{
    // Handle MB lighting sync toggle
    bool isEnabled = m_mbLightingSyncCheck->isChecked();
    // Update lighting sync logic here
}
