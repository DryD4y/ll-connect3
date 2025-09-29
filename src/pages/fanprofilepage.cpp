#include "fanprofilepage.h"
#include <QHeaderView>
#include <QFont>
#include <QTimer>

FanProfilePage::FanProfilePage(QWidget *parent)
    : QWidget(parent)
{
    // Set minimum size for the page - more compact
    setMinimumSize(700, 500);
    
    setupUI();
    setupFanTable();
    setupFanCurve();
    setupControls();
    
    // Start update timer
    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout, this, &FanProfilePage::updateFanData);
    m_updateTimer->start(2000); // Update every 2 seconds
    
    // Initial update
    updateFanData();
}

void FanProfilePage::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(15, 15, 15, 15);
    m_mainLayout->setSpacing(15);
    
    // Header removed to maximize space for fan table
    
    // Content layout - Vertical: Fans on top, Profile on bottom
    m_contentLayout = new QVBoxLayout();
    m_contentLayout->setSpacing(10);
    m_contentLayout->setContentsMargins(0, 0, 0, 0);
    
    m_leftLayout = new QVBoxLayout();
    m_leftLayout->setSpacing(8);
    m_rightLayout = new QVBoxLayout();
    m_rightLayout->setSpacing(8);
    
    // Add both layouts to main content (fans first, then profile)
    m_contentLayout->addLayout(m_leftLayout, 0);  // Fans on top - fixed size
    m_contentLayout->addLayout(m_rightLayout, 1); // Profile on bottom - expandable
    
    m_mainLayout->addLayout(m_contentLayout);
    
    // Header styles removed since header is no longer used
}

void FanProfilePage::setupFanTable()
{
    // Fan section without title to maximize space for the table
    
    m_fanTable = new QTableWidget(4, 6);
    m_fanTable->setObjectName("fanTable");
    
    QStringList headers = {"#", "Port", "Profile", "CPU Temperature", "RPM", "Size"};
    m_fanTable->setHorizontalHeaderLabels(headers);
    
    // Set table properties
    m_fanTable->setAlternatingRowColors(true);
    m_fanTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_fanTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_fanTable->verticalHeader()->setVisible(false);
    m_fanTable->horizontalHeader()->setStretchLastSection(true);
    
    // Set column widths to fit better
    m_fanTable->setColumnWidth(0, 30);  // # column
    m_fanTable->setColumnWidth(1, 80);  // Port column
    m_fanTable->setColumnWidth(2, 80);  // Profile column
    m_fanTable->setColumnWidth(3, 120); // CPU Temperature column
    m_fanTable->setColumnWidth(4, 80);  // RPM column
    m_fanTable->setColumnWidth(5, 80);  // Size column
    
    // Set table size - more compact
    m_fanTable->setMaximumHeight(160);
    m_fanTable->setMinimumHeight(120);
    
    // Populate table data
    for (int row = 0; row < 4; ++row) {
        m_fanTable->setItem(row, 0, new QTableWidgetItem(QString::number(row + 1)));
        m_fanTable->setItem(row, 1, new QTableWidgetItem("Port" + QString::number(row + 1)));
        m_fanTable->setItem(row, 2, new QTableWidgetItem("Quiet"));
        m_fanTable->setItem(row, 3, new QTableWidgetItem("CPU 56°C"));
        m_fanTable->setItem(row, 4, new QTableWidgetItem("975 RPM"));
        
        // Add size combo box
        QComboBox *sizeCombo = new QComboBox();
        sizeCombo->addItems({"120mm", "140mm", "200mm"});
        sizeCombo->setCurrentText("120mm");
        m_fanTable->setCellWidget(row, 5, sizeCombo);
    }
    
    m_leftLayout->addWidget(m_fanTable);
    
    // Style the table
    m_fanTable->setStyleSheet(R"(
        QTableWidget {
            background-color: #2d2d2d;
            border: 1px solid #404040;
            border-radius: 8px;
            gridline-color: #404040;
        }
        
        QTableWidget::item {
            padding: 6px;
            border-bottom: 1px solid #404040;
        }
        
        QTableWidget::item:selected {
            background-color: #2a82da;
        }
        
        QHeaderView::section {
            background-color: #404040;
            color: #ffffff;
            padding: 6px;
            border: none;
            font-weight: bold;
            font-size: 12px;
        }
        
        QComboBox {
            background-color: #404040;
            color: #ffffff;
            border: 1px solid #555555;
            border-radius: 4px;
            padding: 2px;
            font-size: 11px;
        }
    )");
}

void FanProfilePage::setupFanCurve()
{
    m_fanCurveWidget = new QWidget();
    m_fanCurveWidget->setObjectName("fanCurveWidget");
    m_fanCurveWidget->setMinimumSize(400, 180);
    m_fanCurveWidget->setMaximumHeight(220);
    
    // Create a simple placeholder for the fan curve
    QVBoxLayout *curveLayout = new QVBoxLayout(m_fanCurveWidget);
    QLabel *curveLabel = new QLabel("Fan Curve Graph\n(Temperature vs RPM)");
    curveLabel->setObjectName("curveLabel");
    curveLabel->setAlignment(Qt::AlignCenter);
    curveLayout->addWidget(curveLabel);
    
    // Style the widget
    m_fanCurveWidget->setStyleSheet(R"(
        #fanCurveWidget {
            background-color: #1a1a1a;
            border: 1px solid #404040;
            border-radius: 8px;
        }
        
        #curveLabel {
            color: #888888;
            font-size: 14px;
        }
    )");
}

void FanProfilePage::setupControls()
{
    // Profile section without title to save space
    
    // Fan Profile group - use horizontal layout for radio buttons
    m_profileGroup = new QGroupBox("Fan Profile");
    m_profileGroup->setObjectName("controlGroup");
    
    QHBoxLayout *profileLayout = new QHBoxLayout(m_profileGroup);
    profileLayout->setSpacing(15);
    
    m_quietRadio = new QRadioButton("Quiet");
    m_stdSpRadio = new QRadioButton("StdSP");
    m_highSpRadio = new QRadioButton("HighSP");
    m_fullSpRadio = new QRadioButton("FullSP");
    m_customRadio = new QRadioButton("Custom");
    
    m_quietRadio->setChecked(true);
    
    profileLayout->addWidget(m_quietRadio);
    profileLayout->addWidget(m_stdSpRadio);
    profileLayout->addWidget(m_highSpRadio);
    profileLayout->addWidget(m_fullSpRadio);
    profileLayout->addWidget(m_customRadio);
    profileLayout->addStretch();
    
    connect(m_quietRadio, &QRadioButton::toggled, this, &FanProfilePage::onProfileChanged);
    connect(m_stdSpRadio, &QRadioButton::toggled, this, &FanProfilePage::onProfileChanged);
    connect(m_highSpRadio, &QRadioButton::toggled, this, &FanProfilePage::onProfileChanged);
    connect(m_fullSpRadio, &QRadioButton::toggled, this, &FanProfilePage::onProfileChanged);
    connect(m_customRadio, &QRadioButton::toggled, this, &FanProfilePage::onProfileChanged);
    
    m_rightLayout->addWidget(m_profileGroup);
    
    // Add the fan curve widget to the profile section
    m_rightLayout->addWidget(m_fanCurveWidget);
    
    // Add some spacing
    m_rightLayout->addSpacing(5);
    
    // Combined controls layout
    QHBoxLayout *controlsLayout = new QHBoxLayout();
    controlsLayout->setSpacing(20);
    
    // Start/Stop control
    QHBoxLayout *startStopLayout = new QHBoxLayout();
    QLabel *startStopLabel = new QLabel("Start/Stop");
    startStopLabel->setObjectName("controlLabel");
    
    m_startStopCheck = new QCheckBox();
    m_startStopCheck->setObjectName("controlCheck");
    
    startStopLayout->addWidget(startStopLabel);
    startStopLayout->addWidget(m_startStopCheck);
    
    // Temperature and RPM controls
    QHBoxLayout *tempRpmLayout = new QHBoxLayout();
    tempRpmLayout->setSpacing(10);
    
    m_tempLabel = new QLabel("25 °C");
    m_tempLabel->setObjectName("controlLabel");
    
    m_rpmLabel = new QLabel("420 RPM");
    m_rpmLabel->setObjectName("controlLabel");
    
    tempRpmLayout->addWidget(m_tempLabel);
    tempRpmLayout->addWidget(m_rpmLabel);
    
    controlsLayout->addLayout(startStopLayout);
    controlsLayout->addLayout(tempRpmLayout);
    controlsLayout->addStretch();
    
    connect(m_startStopCheck, &QCheckBox::toggled, this, &FanProfilePage::onStartStopToggled);
    
    m_rightLayout->addLayout(controlsLayout);
    
    // Action buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    
    m_applyToAllBtn = new QPushButton("Apply To All");
    m_applyToAllBtn->setObjectName("actionButton");
    
    m_defaultBtn = new QPushButton("Default");
    m_defaultBtn->setObjectName("actionButton");
    
    connect(m_applyToAllBtn, &QPushButton::clicked, this, &FanProfilePage::onApplyToAll);
    connect(m_defaultBtn, &QPushButton::clicked, this, &FanProfilePage::onDefault);
    
    buttonLayout->addWidget(m_applyToAllBtn);
    buttonLayout->addWidget(m_defaultBtn);
    
    m_rightLayout->addLayout(buttonLayout);
    m_rightLayout->addStretch();
    
    // Apply control styles
    setStyleSheet(R"(
        #controlGroup {
            color: #ffffff;
            font-size: 14px;
            font-weight: bold;
            border: 1px solid #404040;
            border-radius: 8px;
            padding: 10px;
            margin-top: 10px;
        }
        
        #controlLabel {
            color: #cccccc;
            font-size: 12px;
        }
        
        #controlCheck {
            color: #ffffff;
        }
        
        QRadioButton {
            color: #cccccc;
            font-size: 12px;
        }
        
        QRadioButton::indicator {
            width: 12px;
            height: 12px;
        }
        
        QRadioButton::indicator::unchecked {
            border: 2px solid #555555;
            border-radius: 6px;
            background-color: transparent;
        }
        
        QRadioButton::indicator::checked {
            border: 2px solid #2a82da;
            border-radius: 6px;
            background-color: #2a82da;
        }
        
        #actionButton {
            background-color: #2a82da;
            color: #ffffff;
            border: none;
            padding: 8px 16px;
            border-radius: 4px;
            margin: 5px;
        }
        
        #actionButton:hover {
            background-color: #1e6bb8;
        }
    )");
}

void FanProfilePage::updateFanCurve()
{
    // Update fan curve based on selected profile
    QString profile = "Quiet";
    if (m_stdSpRadio->isChecked()) profile = "Standard";
    else if (m_highSpRadio->isChecked()) profile = "High Speed";
    else if (m_fullSpRadio->isChecked()) profile = "Full Speed";
    else if (m_customRadio->isChecked()) profile = "Custom";
    
    // Update the curve widget label
    QLabel *curveLabel = m_fanCurveWidget->findChild<QLabel*>("curveLabel");
    if (curveLabel) {
        curveLabel->setText(QString("Fan Curve Graph\n(Temperature vs RPM)\nProfile: %1").arg(profile));
    }
}

void FanProfilePage::updateFanData()
{
    // Simulate fan data updates
    static int counter = 0;
    counter++;
    
    // Update table data
    for (int row = 0; row < 4; ++row) {
        int baseRpm = 950 + (row * 10);
        int rpm = baseRpm + (counter % 50) - 25;
        if (row == 2) rpm = 0; // Port 3 is off
        
        m_fanTable->setItem(row, 3, new QTableWidgetItem("CPU 56°C"));
        m_fanTable->setItem(row, 4, new QTableWidgetItem(QString::number(rpm) + " RPM"));
    }
}

void FanProfilePage::onProfileChanged()
{
    updateFanCurve();
}

void FanProfilePage::onApplyToAll()
{
    // Apply current profile to all fans
    QString profile = "Quiet";
    if (m_stdSpRadio->isChecked()) profile = "StdSP";
    else if (m_highSpRadio->isChecked()) profile = "HighSP";
    else if (m_fullSpRadio->isChecked()) profile = "FullSP";
    else if (m_customRadio->isChecked()) profile = "Custom";
    
    for (int row = 0; row < 4; ++row) {
        m_fanTable->setItem(row, 2, new QTableWidgetItem(profile));
    }
}

void FanProfilePage::onDefault()
{
    m_quietRadio->setChecked(true);
    updateFanCurve();
}

void FanProfilePage::onStartStopToggled()
{
    // Handle start/stop toggle
    bool isRunning = m_startStopCheck->isChecked();
    // Update fan control logic here
}
