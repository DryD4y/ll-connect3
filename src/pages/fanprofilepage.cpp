#include "fanprofilepage.h"
#include <QHeaderView>
#include <QFont>
#include <QTimer>
#include <QVector>
#include <QPointF>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QProcess>
#include <QThread>
#include <QRegularExpression>
#include <QDir>
#include <QRandomGenerator>
#include <cmath>
#include <vector>
#include <algorithm>
#include <deque>
#include <QElapsedTimer>

FanProfilePage::FanProfilePage(QWidget *parent)
    : QWidget(parent)
    , m_cachedTemperature(39) // Start with your current temperature
    , m_temperatureCounter(0)
    , m_cachedFanRPMs(4, 0) // Initialize with 4 fans at 0 RPM
    , m_hidController(nullptr)
{
    // Set minimum size for the page - more compact
    setMinimumSize(700, 500);
    
    setupUI();
    setupFanTable();
    setupFanCurve();
    setupControls();
    
    // Start fast update timer for visual updates
    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout, this, &FanProfilePage::updateFanData);
    m_updateTimer->start(50); // Update every 50ms for smooth real-time updates
    
    // Start slower timer for temperature reading (every 500ms)
    m_tempUpdateTimer = new QTimer(this);
    connect(m_tempUpdateTimer, &QTimer::timeout, this, &FanProfilePage::updateTemperature);
    m_tempUpdateTimer->start(500); // Update temperature every 500ms
    
    // Start timer for fan RPM reading (every 1 second)
    m_fanRPMTimer = new QTimer(this);
    connect(m_fanRPMTimer, &QTimer::timeout, this, &FanProfilePage::updateFanRPMs);
    m_fanRPMTimer->start(1000); // Update fan RPMs every 1 second
    
    // Initialize HID controller for fan control
    m_hidController = new LianLiSLInfinityController();
    if (m_hidController->Initialize()) {
        qDebug() << "Lian Li device connected successfully";
        qDebug() << "Device name:" << QString::fromStdString(m_hidController->GetDeviceName());
        qDebug() << "Firmware version:" << QString::fromStdString(m_hidController->GetFirmwareVersion());
    } else {
        qDebug() << "Failed to connect to Lian Li device - fans will not work";
    }
    
    // Initial update
    updateTemperature();
    updateFanRPMs();
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
    m_fanCurveWidget = new FanCurveWidget();
    m_fanCurveWidget->setObjectName("fanCurveWidget");
    m_fanCurveWidget->setMinimumSize(400, 180);
    m_fanCurveWidget->setMaximumHeight(220);
    
    // Set initial profile
    m_fanCurveWidget->setProfile("Quiet");
    m_fanCurveWidget->setCurrentTemperature(25);
    m_fanCurveWidget->setCurrentRPM(420);
    
    // Style the widget
    m_fanCurveWidget->setStyleSheet(R"(
        #fanCurveWidget {
            background-color: #1a1a1a;
            border: 1px solid #404040;
            border-radius: 8px;
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
    m_customRadio = new QRadioButton("MB RPM Sync");
    
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
    
    // Create horizontal layout for fan curve and buttons
    QHBoxLayout *fanCurveLayout = new QHBoxLayout();
    fanCurveLayout->setSpacing(15);
    
    // Fan curve on the left
    fanCurveLayout->addWidget(m_fanCurveWidget, 1); // Fan curve takes remaining space
    
    // Action buttons stacked vertically on the right, aligned to bottom
    QVBoxLayout *buttonLayout = new QVBoxLayout();
    buttonLayout->setSpacing(10);
    
    m_applyToAllBtn = new QPushButton("Apply To All");
    m_applyToAllBtn->setObjectName("actionButton");
    m_applyToAllBtn->setFixedWidth(120);
    
    m_defaultBtn = new QPushButton("Default");
    m_defaultBtn->setObjectName("actionButton");
    m_defaultBtn->setFixedWidth(120);
    
    connect(m_applyToAllBtn, &QPushButton::clicked, this, &FanProfilePage::onApplyToAll);
    connect(m_defaultBtn, &QPushButton::clicked, this, &FanProfilePage::onDefault);
    
    buttonLayout->addStretch(); // Push buttons to bottom
    buttonLayout->addWidget(m_applyToAllBtn);
    buttonLayout->addWidget(m_defaultBtn);
    
    fanCurveLayout->addLayout(buttonLayout);
    
    m_rightLayout->addLayout(fanCurveLayout);
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
            padding: 10px 16px;
            border-radius: 6px;
            font-size: 12px;
            font-weight: 600;
            min-height: 36px;
        }
        
        #actionButton:hover {
            background-color: #1e6bb8;
        }
        
        #actionButton:pressed {
            background-color: #155a9e;
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
    else if (m_customRadio->isChecked()) profile = "MB RPM Sync";
    
    // Update the fan curve widget
    m_fanCurveWidget->setProfile(profile);
}

void FanProfilePage::updateTemperature()
{
    // Try to get real CPU temperature first, fall back to simulation
    int realTemp = getRealCPUTemperature();
    
    if (realTemp != -1) {
        // Use real temperature
        m_cachedTemperature = realTemp;
    } else {
        // Use simulation with smooth variation
        m_temperatureCounter++;
        int baseTemp = 39; // Your current temperature
        int tempVariation = (m_temperatureCounter % 120) - 60; // -60 to +60 variation
        m_cachedTemperature = qMax(25, qMin(85, baseTemp + tempVariation));
    }
}

void FanProfilePage::updateFanRPMs()
{
    // Try to get real fan RPMs first, fall back to simulation
    QVector<int> realRPMs = getRealFanRPMs();
    
    if (!realRPMs.isEmpty()) {
        // Use real fan RPMs
        m_cachedFanRPMs = realRPMs;
        // qDebug() << "Using real fan RPMs:" << m_cachedFanRPMs;
    } else {
        // Use enhanced simulation with more realistic fan behavior
        static int simulationCounter = 0;
        simulationCounter++;
        
        for (int i = 0; i < 4; ++i) {
            if (i == 2) {
                // Port 3 is off
                m_cachedFanRPMs[i] = 0;
            } else {
                // Simulate realistic fan RPM based on temperature and profile
                int baseRPM = calculateRPMForTemperature(m_cachedTemperature);
                
                // Add more realistic variation based on fan characteristics
                int variation = 0;
                if (baseRPM < 500) {
                    // Low RPM fans have less variation
                    variation = QRandomGenerator::global()->bounded(50) - 25;
                } else if (baseRPM < 1500) {
                    // Medium RPM fans have moderate variation
                    variation = QRandomGenerator::global()->bounded(100) - 50;
                } else {
                    // High RPM fans have more variation
                    variation = QRandomGenerator::global()->bounded(150) - 75;
                }
                
                // Add some "breathing" effect - fans don't change instantly
                int targetRPM = qMax(0, baseRPM + variation);
                
                // Smooth transition to target RPM (simulate fan inertia)
                if (m_cachedFanRPMs[i] == 0) {
                    m_cachedFanRPMs[i] = targetRPM; // Instant start
                } else {
                    int currentRPM = m_cachedFanRPMs[i];
                    int diff = targetRPM - currentRPM;
                    int change = diff / 10; // Gradual change
                    if (change == 0 && diff != 0) {
                        change = (diff > 0) ? 1 : -1; // At least 1 RPM change
                    }
                    m_cachedFanRPMs[i] = qMax(0, currentRPM + change);
                }
                
                // Ensure minimum RPM for running fans (except when off)
                if (m_cachedFanRPMs[i] > 0 && m_cachedFanRPMs[i] < 200) {
                    m_cachedFanRPMs[i] = 200 + QRandomGenerator::global()->bounded(100);
                }
            }
        }
        
        // Debug output every 10 updates
        if (simulationCounter % 10 == 0) {
            // qDebug() << "Simulated fan RPMs (temp:" << m_cachedTemperature << "°C):" << m_cachedFanRPMs;
        }
    }
}

void FanProfilePage::updateFanData()
{
    // Use cached temperature for fast updates
    int currentTemp = m_cachedTemperature;
    
    // Calculate corresponding RPM based on current profile
    int currentRPM = calculateRPMForTemperature(currentTemp);
    
    // Update temperature and RPM labels
    m_tempLabel->setText(QString::number(currentTemp) + " °C");
    m_rpmLabel->setText(QString::number(currentRPM) + " RPM");
    
    // Update fan curve widget - this should trigger a repaint
    m_fanCurveWidget->setCurrentTemperature(currentTemp);
    m_fanCurveWidget->setCurrentRPM(currentRPM);
    
    // Force update of the fan curve widget
    m_fanCurveWidget->update();
    
    // Update table data with real fan RPMs
    for (int row = 0; row < 4; ++row) {
        int rpm = m_cachedFanRPMs[row];
        
        m_fanTable->setItem(row, 3, new QTableWidgetItem("CPU " + QString::number(currentTemp) + "°C"));
        m_fanTable->setItem(row, 4, new QTableWidgetItem(QString::number(rpm) + " RPM"));
    }
    
    // Control fan speeds based on temperature and profile
    controlFanSpeeds();
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
    else if (m_customRadio->isChecked()) profile = "MB RPM Sync";
    
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
    
    if (isRunning) {
        // Test fan speeds directly
        qDebug() << "=== TESTING FAN SPEEDS ===";
        
        // Test Channel 0 (Port 1) - CPU cooler
        qDebug() << "Setting Channel 0 to 20% speed...";
        if (m_hidController) {
            m_hidController->SetChannelSpeed(0, 20);
        }
        QThread::msleep(2000);
        
        qDebug() << "Setting Channel 0 to 50% speed...";
        if (m_hidController) {
            m_hidController->SetChannelSpeed(0, 50);
        }
        QThread::msleep(2000);
        
        qDebug() << "Setting Channel 0 to 80% speed...";
        if (m_hidController) {
            m_hidController->SetChannelSpeed(0, 80);
        }
        QThread::msleep(2000);
        
        // Test Channel 1 (Port 2) - Rear case fan
        qDebug() << "Setting Channel 1 to 30% speed...";
        if (m_hidController) {
            m_hidController->SetChannelSpeed(1, 30);
        }
        QThread::msleep(2000);
        
        // Test Channel 3 (Port 4) - Front case fans
        qDebug() << "Setting Channel 3 to 60% speed...";
        if (m_hidController) {
            m_hidController->SetChannelSpeed(3, 60);
        }
        QThread::msleep(2000);
        
        qDebug() << "Setting all fans to 0% (off)...";
        if (m_hidController) {
            m_hidController->SetChannelSpeed(0, 0);
            m_hidController->SetChannelSpeed(1, 0);
            m_hidController->SetChannelSpeed(3, 0);
        }
        
        qDebug() << "=== FAN TEST COMPLETE ===";
    }
}

int FanProfilePage::calculateRPMForTemperature(int temperature)
{
    // Get current profile
    QString profile = "Quiet";
    if (m_stdSpRadio->isChecked()) profile = "Standard";
    else if (m_highSpRadio->isChecked()) profile = "High Speed";
    else if (m_fullSpRadio->isChecked()) profile = "Full Speed";
    else if (m_customRadio->isChecked()) profile = "MB RPM Sync";
    
    // Define curve data points for each profile (RPM values)
    QVector<QPointF> curvePoints;
    
    if (profile == "Quiet") {
        curvePoints << QPointF(0, 0) << QPointF(25, 420) << QPointF(45, 840) 
                   << QPointF(65, 1050) << QPointF(80, 1680) << QPointF(90, 2100) << QPointF(100, 2100);
    } else if (profile == "Standard") {
        curvePoints << QPointF(0, 200) << QPointF(20, 400) << QPointF(40, 600) 
                   << QPointF(60, 800) << QPointF(75, 1000) << QPointF(85, 1200);
    } else if (profile == "High Speed") {
        curvePoints << QPointF(0, 300) << QPointF(15, 500) << QPointF(35, 800) 
                   << QPointF(55, 1000) << QPointF(70, 1200) << QPointF(80, 1400);
    } else if (profile == "Full Speed") {
        curvePoints << QPointF(0, 400) << QPointF(10, 600) << QPointF(30, 1000) 
                   << QPointF(50, 1400) << QPointF(70, 1800) << QPointF(90, 2200);
    } else if (profile == "MB RPM Sync") {
        curvePoints << QPointF(0, 100) << QPointF(30, 200) << QPointF(50, 400) 
                   << QPointF(70, 600) << QPointF(85, 800) << QPointF(95, 1000);
    } else {
        // Default to Quiet (original Lian Li curve)
        curvePoints << QPointF(0, 0) << QPointF(25, 420) << QPointF(45, 840) 
                   << QPointF(65, 1050) << QPointF(80, 1680) << QPointF(90, 2100) << QPointF(100, 2100);
    }
    
    // Clamp temperature to valid range
    temperature = qMax(0, qMin(100, temperature));
    
    // Find the two points to interpolate between
    for (int i = 0; i < curvePoints.size() - 1; ++i) {
        if (temperature >= curvePoints[i].x() && temperature <= curvePoints[i + 1].x()) {
            // Linear interpolation between the two points
            double t = (temperature - curvePoints[i].x()) / (curvePoints[i + 1].x() - curvePoints[i].x());
            double rpm = curvePoints[i].y() + t * (curvePoints[i + 1].y() - curvePoints[i].y());
            
            // Convert RPM to percentage (assuming max RPM is 1200 for Lian Li fans)
            // For display purposes, we'll return the RPM value
            // For control purposes, we'll convert to percentage in controlFanSpeeds()
            return static_cast<int>(rpm);
        }
    }
    
    // If temperature is outside the curve range, clamp to nearest point
    if (temperature < curvePoints.first().x()) {
        return static_cast<int>(curvePoints.first().y());
    } else {
        return static_cast<int>(curvePoints.last().y());
    }
}

int FanProfilePage::getRealCPUTemperature()
{
    // Use the same temperature reading method as System Info page
    int maxTemp = 0;
    
    // Method 1: Try sensors command first (most accurate) - same as System Info
    QProcess sensorsProcess;
    sensorsProcess.start("sensors", QStringList() << "k10temp-pci-00c3");
    sensorsProcess.waitForFinished(1000);
    
    if (sensorsProcess.exitCode() == 0) {
        QString output = sensorsProcess.readAllStandardOutput();
        // Look for Tctl temperature (CPU core temperature) - same as System Info
        QRegularExpression tempRegex("Tctl:\\s*\\+?([0-9.]+)°C");
        QRegularExpressionMatch match = tempRegex.match(output);
        if (match.hasMatch()) {
            maxTemp = static_cast<int>(match.captured(1).toDouble());
        }
    }
    
    // Method 2: Fallback to hwmon if sensors didn't work - same as System Info
    if (maxTemp == 0) {
        QDir hwmonDir("/sys/class/hwmon");
        QStringList hwmonDirs = hwmonDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        
        for (const QString &hwmon : hwmonDirs) {
            QFile nameFile("/sys/class/hwmon/" + hwmon + "/name");
            if (nameFile.open(QIODevice::ReadOnly)) {
                QTextStream stream(&nameFile);
                QString name = stream.readLine().trimmed();
                nameFile.close();
                
                // Check for CPU temperature sensors - same as System Info
                if (name.contains("coretemp") || name.contains("k10temp") || name.contains("zenpower") || 
                    name.contains("asus") || name.contains("acpi")) {
                    
                    QDir hwmonSubDir("/sys/class/hwmon/" + hwmon);
                    QStringList files = hwmonSubDir.entryList(QDir::Files);
                    for (const QString &file : files) {
                        if (file.startsWith("temp") && file.endsWith("_input")) {
                            QFile tempFile("/sys/class/hwmon/" + hwmon + "/" + file);
                            if (tempFile.open(QIODevice::ReadOnly)) {
                                QTextStream tStream(&tempFile);
                                QString tempStr = tStream.readLine();
                                if (!tempStr.isEmpty()) {
                                    int temp = tempStr.toInt() / 1000; // Convert millidegrees to degrees
                                    if (temp > maxTemp && temp < 200) { // Reasonable temperature range
                                        maxTemp = temp;
                                    }
                                }
                                tempFile.close();
                            }
                        }
                    }
                }
            }
        }
    }
    
    // Method 3: Fallback to thermal zones if hwmon didn't work - same as System Info
    if (maxTemp == 0) {
        QDir thermalDir("/sys/class/thermal");
        QStringList thermalZones = thermalDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const QString &zone : thermalZones) {
            if (zone.startsWith("thermal_zone")) {
                QFile tempFile("/sys/class/thermal/" + zone + "/temp");
                if (tempFile.open(QIODevice::ReadOnly)) {
                    QTextStream stream(&tempFile);
                    int temp = stream.readLine().toInt() / 1000; // Convert millidegrees to degrees
                    if (temp > maxTemp) maxTemp = temp;
                    tempFile.close();
                }
            }
        }
    }
    
    // Return the temperature or -1 to indicate failure
    return maxTemp > 0 ? maxTemp : -1;
}

QVector<int> FanProfilePage::getRealFanRPMs()
{
    QVector<int> fanRPMs;
    
    // Since we're using USB HID controller, we can't read back fan speeds
    // Instead, we'll use the last known target RPM from our control logic
    // This is stored in the static variable in controlFanSpeeds()
    
    // For now, return empty vector - the UI will show the target RPM instead
    // In a real implementation, you might want to store the last set RPM values
    // in member variables and return them here
    
    return fanRPMs;
}

void FanProfilePage::controlFanSpeeds()
{
    if (!m_hidController) {
        return;
    }
    
    // Get current CPU temperature
    int currentTemp = m_cachedTemperature;
    
    // --- state (static inside controlFanSpeeds or make members) ---
    static double Tf = 0.0;                // filtered temp
    static std::deque<double> hist;        // 1.5 s ring buffer of Tf
    static int rpm_out = 0;
    static QElapsedTimer stepTimer;
    
    double dt = stepTimer.isValid() ? stepTimer.restart()/1000.0 : 0.1;
    if (dt <= 0) dt = 0.1;

    // 1) Asymmetric filter
    int Traw = currentTemp;  // your measured temp
    double alpha = (Traw >= Tf) ? 0.60 : 0.15;
    Tf += alpha * (Traw - Tf);

    // Keep ~1.5 s history for robust derivative (assumes ~10 Hz loop)
    int histMax = std::max(2, int(std::round(1.5 / dt)));
    hist.push_back(Tf);
    while ((int)hist.size() > histMax) hist.pop_front();

    // 2) Positive dT/dt (°C/s), clamped
    double dTdt = 0.0;
    if (hist.size() >= 2) dTdt = (hist.back() - hist.front()) / std::max(0.3, dt*(hist.size()-1));
    if (dTdt < 0) dTdt = 0;
    if (dTdt > 3.0) dTdt = 3.0;

    // A) Only preheat when heating
    const bool heating = (dTdt > 0.05);   // small floor to avoid noise

    int base_now  = calculateRPMForTemperature(int(std::round(Tf)));
    int base_pred = calculateRPMForTemperature(int(std::round(Tf + dTdt * 7.0)));

    int base_rpm  = heating ? std::max(base_now, base_pred) : base_now;
    int ff_rpm    = heating ? int(std::round(dTdt * 180.0)) : 0;
    
    // 4) Spike booster when heating rapidly
    static QElapsedTimer spikeT; static bool spike=false;
    if (heating && dTdt > 1.2 && !spike) { spike=true; spikeT.restart(); }
    int spikeBoost = 0;
    if (spike) {
        int ms = spikeT.elapsed();
        if (ms < 12000) spikeBoost = int(std::round(300.0 * (1.0 - ms/12000.0)));
        else spike = false;
    }
    
    // Clear spike/boost when not heating
    if (!heating) { spike = false; }

    int target = std::clamp(base_rpm + ff_rpm + spikeBoost, 0, 2100);

    // C) Below-50°C governor (explicit requirement)
    static bool coolBand = false;
    const double TLOW = 50.0, HYST = 0.7;
    
    if (!coolBand && Tf <= TLOW) coolBand = true;
    if (coolBand && Tf >= TLOW + HYST) coolBand = false;
    
    if (coolBand) {
        // no look-ahead, no spike, no hold
        int lowTarget = calculateRPMForTemperature(int(std::round(Tf))); // uses actual curve
        target = std::max(lowTarget, 800); // clamp to quiet floor
    }

    // B) Cooling session that actually finishes (ignores deadband while active)
    const int   DOWN_DB     = 30;      // threshold to start a down-session
    const int   DOWN_EPS    = 8;       // finish when within this of target
    const double DOWN_SLEW  = 60.0;    // RPM/s downward
    const int   HOLD_MS     = 8000;    // cooldown hold before any drop
    const int   up_db       = 5;       // very small on rise
    const double up_slew    = 350.0;   // RPM/s upward

    // state
    static bool coolingSession = false;
    static bool holdDown = false;
    static QElapsedTimer holdT;

    // hold management - prevent rapid drops after heating spikes
    // Hold should only prevent increases, not block decreases
    if (target >= rpm_out) { 
        holdDown = false; 
        coolingSession = false; 
    } else { // target < rpm_out
        if (!holdDown && !coolingSession) { 
            holdDown = true; 
            holdT.restart(); 
        }
        // Always allow cooling down, even while on hold
        // Hold only prevents bouncing back up too soon
        if (holdDown && holdT.elapsed() > 7000) { // Reduced from 15s to 7s
            holdDown = false;
        }
    }
    
    // Force release hold when clearly cool
    if (Tf < 49.5) {
        holdDown = false;
    }

    // start session only when allowed AND gap is meaningful
    if (!heating && !holdDown && !coolingSession && target < rpm_out - DOWN_DB) {
        coolingSession = true;
    }

    // apply session
    int maxStepDown = std::max(1, int(std::round(DOWN_SLEW * dt)));
    int maxStepUp = std::max(1, int(std::round(up_slew * dt)));
    int gated = rpm_out;

    if (heating) {
        coolingSession = false; // abort immediately if temp rising
        if (target - rpm_out >= up_db) {
            gated = std::min(target, rpm_out + maxStepUp);
        } else {
            gated = rpm_out; // within deadband
        }
    } else if (target < rpm_out) {
        if (coolingSession) {
            // ignore deadband while in session; staircase to target
            int next = std::max(target, rpm_out - maxStepDown);
            if ((rpm_out - next) <= DOWN_EPS || next <= target + DOWN_EPS) {
                coolingSession = false; // done
            }
            gated = next;
        } else {
            // Allow cooling down even while on hold - hold only prevents increases
            gated = std::max(target, rpm_out - maxStepDown);
        }
    } else {
        // equal -> keep
        gated = rpm_out;
    }

    // 5) Separate write threshold for down steps (tiny during session)
    int writeThreshUpRPM   = 20;  // e.g., 20–30 RPM
    int writeThreshDownRPM = coolingSession ? 0 : 20;

    bool shouldWrite = false;
    static bool wasHoldDown = false;
    bool holdJustReleased = (wasHoldDown && !holdDown);
    
    if (gated > rpm_out) {
        shouldWrite = (gated - rpm_out) >= writeThreshUpRPM;
    } else if (gated < rpm_out) {
        shouldWrite = (rpm_out - gated) >= writeThreshDownRPM;
    } else {
        shouldWrite = (rpm_out == 0); // always allow first write
    }
    
    // Always write when hold is released
    if (holdJustReleased) {
        shouldWrite = true;
    }
    
    wasHoldDown = holdDown;

    if (shouldWrite) {
        // write to all used ports (including Port 0 which showed activity in kernel status)
        if (gated > 0) {
            setFanSpeed(0, gated); // Port 0 - might be the main fan port
            setFanSpeed(1, gated);
            setFanSpeed(2, gated);
            setFanSpeed(4, gated);
        } else {
            setFanSpeed(0, 0);
            setFanSpeed(1, 0); setFanSpeed(2, 0); setFanSpeed(4, 0);
        }
        rpm_out = gated;
        qDebug() << "WRITE: heating=" << heating << " coolBand=" << coolBand 
                 << " session=" << coolingSession << " hold=" << holdDown
                 << " Tf=" << Tf << " dTdt=" << dTdt << " rpm_out=" << rpm_out 
                 << " target=" << target << " gated=" << gated;
    } else {
        qDebug() << "HOLD:  heating=" << heating << " coolBand=" << coolBand 
                 << " session=" << coolingSession << " hold=" << holdDown
                 << " Tf=" << Tf << " dTdt=" << dTdt << " rpm_out=" << rpm_out 
                 << " target=" << target << " gated=" << gated;
    }
}

void FanProfilePage::setFanSpeed(int port, int targetRPM)
{
    // Clamp speed to valid range (0-2100 based on real data)
    targetRPM = qBound(0, targetRPM, 2100);
    
    if (!m_hidController) {
        qDebug() << "HID controller not available for Port" << port;
        return;
    }
    
    qDebug() << "HID controller available, attempting to set fan speed...";
    
    // Convert RPM to percentage for kernel driver
    // Based on your calibration: 100% only gives us 59.4 dBA (~1900 RPM)
    // Convert RPM to percentage (0-100%)
    int speedPercent = (targetRPM * 100) / 2100;
    
    // Clamp to reasonable range
    speedPercent = qBound(0, speedPercent, 100);
    
    // Debug: show what we're actually sending
    qDebug() << "RPM conversion: targetRPM=" << targetRPM << " -> speedPercent=" << speedPercent << "%";
    
    // Additional debug: show expected dBA
    double expectedDBA = 36.8 + (targetRPM - 800) * (62.9 - 36.8) / (2100 - 800);
    qDebug() << "Expected dBA for" << targetRPM << "RPM:" << expectedDBA;
    
    // Convert port number to channel (Port 1 = Channel 0, Port 2 = Channel 1, etc.)
    uint8_t channel = port - 1;
    
    // Use USB HID controller instead of kernel module
    bool success = m_hidController->SetChannelSpeed(channel, speedPercent);
    
    if (success) {
        qDebug() << "Set Port" << port << "(Channel" << channel << ") to" << targetRPM << "RPM (" << speedPercent << "%, expected dBA=" << (36.8 + (targetRPM - 800) * (62.9 - 36.8) / (2100 - 800)) << ")";
    } else {
        qDebug() << "Failed to set Port" << port << "(Channel" << channel << ") to" << targetRPM << "RPM via USB HID";
    }
}
