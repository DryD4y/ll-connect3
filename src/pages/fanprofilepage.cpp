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
    , m_cachedCPULoad(0) // Initialize CPU load
    , m_cachedGPULoad(0) // Initialize GPU load
    , m_cachedFanRPMs(4, 0) // Initialize with 4 fans at 0 RPM
    , m_portConnected(4, false) // Initialize port detection
    , m_activePorts() // Empty initially
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
    
    // Start timer for CPU load reading (every 500ms)
    m_cpuLoadTimer = new QTimer(this);
    connect(m_cpuLoadTimer, &QTimer::timeout, this, &FanProfilePage::updateCPULoad);
    m_cpuLoadTimer->start(500); // Update CPU load every 500ms
    
    // Start timer for GPU load reading (every 500ms)
    m_gpuLoadTimer = new QTimer(this);
    connect(m_gpuLoadTimer, &QTimer::timeout, this, &FanProfilePage::updateGPULoad);
    m_gpuLoadTimer->start(500); // Update GPU load every 500ms
    
    // Initialize HID controller for fan control
    m_hidController = new LianLiSLInfinityController();
    if (m_hidController->Initialize()) {
        qDebug() << "Lian Li device connected successfully";
        qDebug() << "Device name:" << QString::fromStdString(m_hidController->GetDeviceName());
        qDebug() << "Firmware version:" << QString::fromStdString(m_hidController->GetFirmwareVersion());
    } else {
        qDebug() << "Failed to connect to Lian Li device - fans will not work";
    }
    
    // Detect connected ports
    detectConnectedPorts();
    
    // Initial update
    updateTemperature();
    updateFanRPMs();
    updateCPULoad();
    updateGPULoad();
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
    
    m_fanTable = new QTableWidget(0, 7); // Start with 0 rows, will be populated dynamically
    m_fanTable->setObjectName("fanTable");
    
    QStringList headers = {"#", "Port", "Profile", "CPU Temperature", "GPU Load", "Fan RPMs", "Size"};
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
    m_fanTable->setColumnWidth(4, 80);  // GPU Load column
    m_fanTable->setColumnWidth(5, 80);  // Fan RPMs column
    m_fanTable->setColumnWidth(6, 60);  // Size column (smaller)
    
    // Set table size - more compact
    m_fanTable->setMaximumHeight(160);
    m_fanTable->setMinimumHeight(120);
    
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
    
    // Refresh ports button
    QPushButton *refreshPortsBtn = new QPushButton("Refresh Ports");
    refreshPortsBtn->setObjectName("refreshPortsBtn");
    refreshPortsBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #4a4a4a;
            color: #ffffff;
            border: 1px solid #666666;
            border-radius: 4px;
            padding: 8px 16px;
            font-size: 12px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #5a5a5a;
            border-color: #888888;
        }
        QPushButton:pressed {
            background-color: #3a3a3a;
        }
    )");
    connect(refreshPortsBtn, &QPushButton::clicked, this, &FanProfilePage::detectConnectedPorts);
    controlsLayout->addWidget(refreshPortsBtn);
    
    connect(m_startStopCheck, &QCheckBox::toggled, this, &FanProfilePage::onStartStopToggled);
    
    m_rightLayout->addLayout(controlsLayout);
    
    // Create horizontal layout for fan curve and buttons
    QHBoxLayout *fanCurveLayout = new QHBoxLayout();
    fanCurveLayout->setSpacing(15);
    
    // Fan curve on the left
    fanCurveLayout->addWidget(m_fanCurveWidget, 1); // Fan curve takes remaining space
    
    // Buttons removed - they weren't doing anything useful
    
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
    
    // Calculate corresponding RPM based on current profile (for curve reference)
    int calculatedRPM = calculateRPMForTemperature(currentTemp);
    
    // Get average real RPM from connected fans
    int realRPM = 0;
    if (!m_activePorts.isEmpty()) {
        int totalRPM = 0;
        int activeCount = 0;
        for (int port : m_activePorts) {
            int portRPM = getRealFanRPM(port);
            totalRPM += portRPM;
            if (portRPM > 0) activeCount++;
        }
        realRPM = activeCount > 0 ? totalRPM / activeCount : 0;
    }
    
    // Update temperature, load, and RPM labels
    m_tempLabel->setText(QString::number(currentTemp) + " °C");
    m_rpmLabel->setText(QString::number(realRPM) + " RPM (CPU:" + QString::number(m_cachedCPULoad) + "%, GPU:" + QString::number(m_cachedGPULoad) + "%)");
    
    // Update fan curve widget - show real RPM instead of calculated
    m_fanCurveWidget->setCurrentTemperature(currentTemp);
    m_fanCurveWidget->setCurrentRPM(realRPM);
    
    // Force update of the fan curve widget
    m_fanCurveWidget->update();
    
    // Update table data with real fan RPMs (only for connected ports)
    for (int row = 0; row < m_activePorts.size(); ++row) {
        int port = m_activePorts[row];
        int realRPM = getRealFanRPM(port); // Get actual RPM from kernel driver
        
        m_fanTable->setItem(row, 3, new QTableWidgetItem("CPU " + QString::number(currentTemp) + "°C (L:" + QString::number(m_cachedCPULoad) + "%)"));
        m_fanTable->setItem(row, 4, new QTableWidgetItem("GPU " + QString::number(m_cachedGPULoad) + "%"));
        m_fanTable->setItem(row, 5, new QTableWidgetItem(QString::number(realRPM) + " RPM"));
    }
    
    // Control fan speeds based on temperature and profile
    controlFanSpeeds();
}

void FanProfilePage::onProfileChanged()
{
    updateFanCurve();
}

// Button functions removed - they weren't doing anything useful

void FanProfilePage::onStartStopToggled()
{
    // Handle start/stop toggle
    bool isRunning = m_startStopCheck->isChecked();
    
    if (isRunning) {
        // Test fan speeds directly - only test connected ports
        qDebug() << "=== TESTING CONNECTED FAN PORTS ===";
        qDebug() << "Active ports:" << m_activePorts;
        
        if (m_activePorts.isEmpty()) {
            qDebug() << "No connected ports found - skipping fan test";
            return;
        }
        
        // Test each connected port
        for (int port : m_activePorts) {
            qDebug() << "Testing Port" << port << "...";
            
            // Test 20% speed
            qDebug() << "Setting Port" << port << "to 20% speed...";
            if (m_hidController) {
                m_hidController->SetChannelSpeed(port - 1, 20);
            }
            QThread::msleep(2000);
            
            // Test 50% speed
            qDebug() << "Setting Port" << port << "to 50% speed...";
            if (m_hidController) {
                m_hidController->SetChannelSpeed(port - 1, 50);
            }
            QThread::msleep(2000);
            
            // Test 80% speed
            qDebug() << "Setting Port" << port << "to 80% speed...";
            if (m_hidController) {
                m_hidController->SetChannelSpeed(port - 1, 80);
            }
            QThread::msleep(2000);
        }
        
        // Turn off all connected fans
        qDebug() << "Turning off all connected fans...";
        for (int port : m_activePorts) {
            if (m_hidController) {
                m_hidController->SetChannelSpeed(port - 1, 0);
            }
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
    QVector<int> fanRPMs(4, 0); // Initialize with 4 ports, all at 0 RPM
    
    // Read real RPMs from kernel driver for all ports
    for (int port = 1; port <= 4; ++port) {
        fanRPMs[port - 1] = getRealFanRPM(port);
    }
    
    return fanRPMs;
}

int FanProfilePage::getRealFanRPM(int port)
{
    if (port < 1 || port > 4) {
        return 0;
    }
    
    // Read RPM from kernel driver proc file
    QString procPath = QString("/proc/Lian_li_SL_INFINITY/Port_%1/fan_speed").arg(port);
    QFile file(procPath);
    
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to open" << procPath << "for reading";
        return 0;
    }
    
    QTextStream stream(&file);
    QString rpmStr = stream.readLine().trimmed();
    file.close();
    
    bool ok;
    int percentage = rpmStr.toInt(&ok);
    
    if (!ok) {
        qDebug() << "Failed to parse percentage from" << procPath << ":" << rpmStr;
        return 0;
    }
    
    // Convert percentage to actual RPM
    int rpm = convertPercentageToRPM(percentage);
    
    return rpm;
}

int FanProfilePage::convertPercentageToRPM(int percentage)
{
    // Convert kernel driver percentage (0-100%) to RPM values
    // Simple linear mapping: 0% = 0 RPM, 100% = 2100 RPM
    
    if (percentage <= 0) return 0;
    if (percentage >= 100) return 2100;
    
    // Linear interpolation between 0-100% and 0-2100 RPM
    int rpm = (percentage * 2100) / 100;
    
    return rpm;
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

    int base_now  = calculateRPMForLoad(int(std::round(Tf)), m_cachedCPULoad, m_cachedGPULoad);
    int base_pred = calculateRPMForLoad(int(std::round(Tf + dTdt * 7.0)), m_cachedCPULoad, m_cachedGPULoad);

    int base_rpm  = heating ? std::max(base_now, base_pred) : base_now;
    int ff_rpm    = heating ? int(std::round(dTdt * 300.0)) : 0; // Increased from 180 to 300
    
    // 4) Spike booster when heating rapidly
    static QElapsedTimer spikeT; static bool spike=false;
    if (heating && dTdt > 1.2 && !spike) { spike=true; spikeT.restart(); }
    int spikeBoost = 0;
    if (spike) {
        int ms = spikeT.elapsed();
        if (ms < 15000) spikeBoost = int(std::round(500.0 * (1.0 - ms/15000.0))); // Increased boost and duration
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
        int lowTarget = calculateRPMForLoad(int(std::round(Tf)), m_cachedCPULoad, m_cachedGPULoad); // uses actual curve
        target = std::max(lowTarget, 800); // clamp to quiet floor
    }

    // B) Cooling session that actually finishes (ignores deadband while active)
    // Make cooling more aggressive at higher temperatures
    int DOWN_DB = 20;      // threshold to start a down-session
    int DOWN_EPS = 5;       // finish when within this of target
    double DOWN_SLEW = 120.0;   // RPM/s downward
    int HOLD_MS = 5000;    // cooldown hold before any drop
    const int   up_db       = 2;       // very small on rise (reduced from 5)
    const double up_slew    = 800.0;   // RPM/s upward (increased from 350)
    
    // More aggressive cooling at higher temperatures
    double up_slew_adaptive = up_slew;  // Start with base upward slew rate
    
    if (Tf > 70.0) {
        DOWN_DB = 10;       // Start cooling sooner at high temps
        DOWN_EPS = 3;       // More precise cooling
        DOWN_SLEW = 200.0;  // Much faster cooling at high temps
        HOLD_MS = 2000;     // Shorter hold time at high temps
        up_slew_adaptive = 1200.0;  // Very fast upward response at high temps
    } else if (Tf > 60.0) {
        DOWN_DB = 15;       // Moderate cooling
        DOWN_EPS = 4;       // Moderate precision
        DOWN_SLEW = 150.0;  // Moderate speed
        HOLD_MS = 3000;     // Moderate hold time
        up_slew_adaptive = 1000.0;  // Fast upward response at medium temps
    } else if (Tf > 50.0) {
        up_slew_adaptive = 900.0;   // Moderate upward response
    }

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
        // Shorter hold times at higher temperatures
        int holdTime = (Tf > 70.0) ? 1000 : (Tf > 60.0) ? 2000 : 3000;
        if (holdDown && holdT.elapsed() > holdTime) {
            holdDown = false;
        }
    }
    
    // Force release hold when clearly cool or at high temps
    if (Tf < 49.5 || Tf > 75.0) {
        holdDown = false;
    }

    // start session only when allowed AND gap is meaningful
    if (!heating && !holdDown && !coolingSession && target < rpm_out - DOWN_DB) {
        coolingSession = true;
    }

    // apply session
    int maxStepDown = std::max(1, int(std::round(DOWN_SLEW * dt)));
    int maxStepUp = std::max(1, int(std::round(up_slew_adaptive * dt)));
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
    int writeThreshUpRPM   = (Tf > 60.0) ? 5 : 10;  // More responsive upward at higher temps
    int writeThreshDownRPM = coolingSession ? 0 : (Tf > 70.0) ? 10 : 20; // More responsive at high temps

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
        // write to only connected ports
        for (int port : m_activePorts) {
            setFanSpeed(port, gated);
        }
        rpm_out = gated;
        qDebug() << "WRITE: heating=" << heating << " coolBand=" << coolBand 
                 << " session=" << coolingSession << " hold=" << holdDown
                 << " Tf=" << Tf << " dTdt=" << dTdt << " rpm_out=" << rpm_out 
                 << " target=" << target << " gated=" << gated
                 << " activePorts=" << m_activePorts;
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
    
    // Convert RPM to percentage for kernel driver
    // Based on our calibration: 100% gives us ~1900 RPM (59.4 dBA)
    // Use a more aggressive mapping to hit 100% at lower RPM
    int speedPercent;
    if (targetRPM <= 0) {
        speedPercent = 0;
    } else if (targetRPM <= 800) {
        // 0-800 RPM maps to 0-20%
        speedPercent = (targetRPM * 20) / 800;
    } else if (targetRPM <= 1500) {
        // 800-1500 RPM maps to 20-70%
        speedPercent = 20 + ((targetRPM - 800) * 50) / 700;
    } else {
        // 1500-2100 RPM maps to 70-100%
        speedPercent = 70 + ((targetRPM - 1500) * 30) / 600;
    }
    
    // Clamp to reasonable range
    speedPercent = qBound(0, speedPercent, 100);
    
    // Debug: show what we're actually sending
    qDebug() << "RPM conversion: targetRPM=" << targetRPM << " -> speedPercent=" << speedPercent << "%";
    
    // Additional debug: show expected dBA
    double expectedDBA = 36.8 + (targetRPM - 800) * (62.9 - 36.8) / (2100 - 800);
    qDebug() << "Expected dBA for" << targetRPM << "RPM:" << expectedDBA;
    
    // Use kernel driver for individual port control (more reliable)
    QString procPath = QString("/proc/Lian_li_SL_INFINITY/Port_%1/fan_speed").arg(port);
    QFile file(procPath);
    
    if (file.open(QIODevice::WriteOnly)) {
        QTextStream stream(&file);
        stream << speedPercent;
        file.close();
        
        qDebug() << "Set Port" << port << "to" << targetRPM << "RPM (" << speedPercent << "%, expected dBA=" << expectedDBA << ") via kernel driver";
    } else {
        qDebug() << "Failed to open" << procPath << "for writing - falling back to USB HID";
        
        // Fallback to USB HID controller if kernel driver fails
        if (m_hidController) {
            uint8_t channel = port - 1;
            bool success = m_hidController->SetChannelSpeed(channel, speedPercent);
            
            if (success) {
                qDebug() << "Set Port" << port << "(Channel" << channel << ") to" << targetRPM << "RPM (" << speedPercent << "%, expected dBA=" << expectedDBA << ") via USB HID fallback";
            } else {
                qDebug() << "Failed to set Port" << port << "(Channel" << channel << ") to" << targetRPM << "RPM via USB HID fallback";
            }
        } else {
            qDebug() << "HID controller not available for Port" << port;
        }
    }
}

void FanProfilePage::updateCPULoad()
{
    // Try to get real CPU load first, fall back to simulation
    int realLoad = getRealCPULoad();
    
    if (realLoad != -1) {
        // Use real CPU load
        m_cachedCPULoad = realLoad;
    } else {
        // Use simulation with some variation
        static int loadCounter = 0;
        loadCounter++;
        
        // Simulate CPU load based on temperature (higher temp = higher load)
        int baseLoad = (m_cachedTemperature > 60) ? 40 : 20;
        int variation = (loadCounter % 80) - 40; // -40 to +40 variation
        m_cachedCPULoad = qMax(0, qMin(100, baseLoad + variation));
    }
}

void FanProfilePage::updateGPULoad()
{
    // Try to get real GPU load first, fall back to simulation
    int realLoad = getRealGPULoad();
    
    if (realLoad != -1) {
        // Use real GPU load
        m_cachedGPULoad = realLoad;
    } else {
        // Use simulation with some variation
        static int loadCounter = 0;
        loadCounter++;
        
        // Simulate GPU load (usually lower than CPU)
        int baseLoad = (m_cachedTemperature > 65) ? 30 : 10;
        int variation = (loadCounter % 60) - 30; // -30 to +30 variation
        m_cachedGPULoad = qMax(0, qMin(100, baseLoad + variation));
    }
}

int FanProfilePage::getRealCPULoad()
{
    // Method 1: Try /proc/loadavg (1-minute average)
    QFile loadavgFile("/proc/loadavg");
    if (loadavgFile.open(QIODevice::ReadOnly)) {
        QTextStream stream(&loadavgFile);
        QString line = stream.readLine();
        loadavgFile.close();
        
        QStringList parts = line.split(' ');
        if (parts.size() >= 3) {
            double load1min = parts[0].toDouble();
            // Convert load average to percentage (rough approximation)
            // Assuming 4 cores, load > 4.0 = 100%
            int loadPercent = qMin(100, static_cast<int>((load1min / 4.0) * 100));
            return loadPercent;
        }
    }
    
    // Method 2: Try /proc/stat (more accurate)
    QFile statFile("/proc/stat");
    if (statFile.open(QIODevice::ReadOnly)) {
        QTextStream stream(&statFile);
        QString line = stream.readLine();
        statFile.close();
        
        if (line.startsWith("cpu ")) {
            QStringList parts = line.split(' ', Qt::SkipEmptyParts);
            if (parts.size() >= 8) {
                // Parse CPU times: user, nice, system, idle, iowait, irq, softirq, steal
                qint64 user = parts[1].toLongLong();
                qint64 nice = parts[2].toLongLong();
                qint64 system = parts[3].toLongLong();
                qint64 idle = parts[4].toLongLong();
                qint64 iowait = parts[5].toLongLong();
                qint64 irq = parts[6].toLongLong();
                qint64 softirq = parts[7].toLongLong();
                qint64 steal = parts.size() > 8 ? parts[8].toLongLong() : 0;
                
                qint64 totalIdle = idle + iowait;
                qint64 totalNonIdle = user + nice + system + irq + softirq + steal;
                qint64 total = totalIdle + totalNonIdle;
                
                // Calculate CPU usage percentage
                static qint64 prevTotal = 0;
                static qint64 prevIdle = 0;
                
                if (prevTotal > 0) {
                    qint64 totalDiff = total - prevTotal;
                    qint64 idleDiff = totalIdle - prevIdle;
                    
                    if (totalDiff > 0) {
                        int cpuPercent = static_cast<int>(((totalDiff - idleDiff) * 100) / totalDiff);
                        prevTotal = total;
                        prevIdle = totalIdle;
                        return qMax(0, qMin(100, cpuPercent));
                    }
                }
                
                prevTotal = total;
                prevIdle = totalIdle;
            }
        }
    }
    
    return -1; // Failed to get CPU load
}

int FanProfilePage::getRealGPULoad()
{
    // Method 1: Try nvidia-smi (NVIDIA GPUs)
    QProcess nvidiaProcess;
    nvidiaProcess.start("nvidia-smi", QStringList() << "--query-gpu=utilization.gpu" << "--format=csv,noheader,nounits");
    nvidiaProcess.waitForFinished(1000);
    
    if (nvidiaProcess.exitCode() == 0) {
        QString output = nvidiaProcess.readAllStandardOutput().trimmed();
        bool ok;
        int gpuLoad = output.toInt(&ok);
        if (ok && gpuLoad >= 0 && gpuLoad <= 100) {
            return gpuLoad;
        }
    }
    
    // Method 2: Try radeontop (AMD GPUs)
    QProcess radeonProcess;
    radeonProcess.start("radeontop", QStringList() << "-d" << "1" << "-l" << "1");
    radeonProcess.waitForFinished(2000);
    
    if (radeonProcess.exitCode() == 0) {
        QString output = radeonProcess.readAllStandardOutput();
        QRegularExpression gpuRegex("gpu\\s+(\\d+)%");
        QRegularExpressionMatch match = gpuRegex.match(output);
        if (match.hasMatch()) {
            return match.captured(1).toInt();
        }
    }
    
    // Method 3: Try intel_gpu_top (Intel GPUs)
    QProcess intelProcess;
    intelProcess.start("intel_gpu_top", QStringList() << "-s" << "1");
    intelProcess.waitForFinished(2000);
    
    if (intelProcess.exitCode() == 0) {
        QString output = intelProcess.readAllStandardOutput();
        QRegularExpression gpuRegex("GPU\\s+(\\d+)%");
        QRegularExpressionMatch match = gpuRegex.match(output);
        if (match.hasMatch()) {
            return match.captured(1).toInt();
        }
    }
    
    return -1; // Failed to get GPU load
}

int FanProfilePage::calculateRPMForLoad(int temperature, int cpuLoad, int gpuLoad)
{
    // Get base RPM from temperature
    int baseRPM = calculateRPMForTemperature(temperature);
    
    // Calculate load-based boost
    int maxLoad = qMax(cpuLoad, gpuLoad);
    int loadBoost = 0;
    
    if (maxLoad > 80) {
        // High load: significant boost
        loadBoost = 300 + ((maxLoad - 80) * 10); // 300-500 RPM boost
    } else if (maxLoad > 60) {
        // Medium load: moderate boost
        loadBoost = 150 + ((maxLoad - 60) * 7); // 150-290 RPM boost
    } else if (maxLoad > 40) {
        // Low load: small boost
        loadBoost = (maxLoad - 40) * 5; // 0-100 RPM boost
    }
    
    // Apply load boost to base RPM
    int finalRPM = baseRPM + loadBoost;
    
    // Clamp to valid range
    finalRPM = qMax(0, qMin(2100, finalRPM));
    
    qDebug() << "Load-based RPM: temp=" << temperature << "°C, CPU=" << cpuLoad << "%, GPU=" << gpuLoad 
             << "%, baseRPM=" << baseRPM << ", loadBoost=" << loadBoost << ", finalRPM=" << finalRPM;
    
    return finalRPM;
}

// Profile test function removed - no longer needed

void FanProfilePage::detectConnectedPorts()
{
    qDebug() << "=== DETECTING CONNECTED PORTS ===";
    
    // Reset port status
    m_portConnected.fill(false);
    m_activePorts.clear();
    
    // Test each port using kernel driver for individual fan control
    // This allows us to test each fan individually using the updated protocol
    
    for (int port = 1; port <= 4; ++port) {
        qDebug() << "Testing Port" << port << " with individual fan control...";
        
        // Get initial RPM value
        int initialRPM = getRealFanRPM(port);
        qDebug() << "  Initial RPM:" << initialRPM;
        
        // Use kernel driver for individual fan control
        // This should control only the specific fan on this port
        QString procPath = QString("/proc/Lian_li_SL_INFINITY/Port_%1/fan_speed").arg(port);
        QFile file(procPath);
        
        bool success = false;
        if (file.open(QIODevice::WriteOnly)) {
            QTextStream stream(&file);
            stream << 30; // Set to 30% speed
            file.close();
            success = true;
        }
        
        qDebug() << "  Individual control success:" << success;
        
        if (success) {
            // Wait for fan to respond
            QThread::msleep(1000);
            
            // Check if RPM changed (individual fan should respond)
            int newRPM = getRealFanRPM(port);
            qDebug() << "  After individual control RPM:" << newRPM;
            qDebug() << "  RPM change:" << (newRPM - initialRPM);
            
            // Port is connected if RPM changed significantly
            bool rpmChanged = (abs(newRPM - initialRPM) > 50);
            
            if (rpmChanged) {
                qDebug() << "✓ Port" << port << "is connected (individual fan responded)";
                m_portConnected[port - 1] = true;
                m_activePorts.append(port);
            } else {
                qDebug() << "✗ Port" << port << "is not connected (no individual response)";
                m_portConnected[port - 1] = false;
            }
        } else {
            qDebug() << "✗ Port" << port << "is not connected (individual control failed)";
            m_portConnected[port - 1] = false;
        }
        
        // Small delay between tests
        QThread::msleep(500);
    }
    
    // Fallback: If no ports were detected, use the known working setup
    // Since individual fan control might not work with this hardware,
    // we'll assume all ports have fans and let the user control them globally
    if (m_activePorts.isEmpty()) {
        qDebug() << "No ports detected with individual control, using fallback for your setup (Ports 1, 2, 4)";
        
        // Set up the known connected ports as fallback
        m_portConnected[0] = true;  // Port 1
        m_portConnected[1] = true;  // Port 2  
        m_portConnected[2] = false; // Port 3 (no fan)
        m_portConnected[3] = true;  // Port 4
        
        m_activePorts.append(1);
        m_activePorts.append(2);
        m_activePorts.append(4);
        
        qDebug() << "Fallback detected ports:" << m_activePorts;
    }
    
    // Turn off all fans after detection
    for (int port : m_activePorts) {
        QString procPath = QString("/proc/Lian_li_SL_INFINITY/Port_%1/fan_speed").arg(port);
        QFile file(procPath);
        if (file.open(QIODevice::WriteOnly)) {
            QTextStream stream(&file);
            stream << 0; // Turn off
            file.close();
        }
    }
    
    qDebug() << "Final result - Active ports:" << m_activePorts;
    qDebug() << "Port connection status:" << m_portConnected;
    qDebug() << "=== PORT DETECTION COMPLETE ===";
    
    // Update the table with only connected ports
    updateFanTable();
}

void FanProfilePage::updateFanTable()
{
    if (!m_fanTable) return;
    
    // Clear existing rows
    m_fanTable->setRowCount(0);
    
    // Add rows only for connected ports
    int row = 0;
    for (int port : m_activePorts) {
        m_fanTable->insertRow(row);
        
        // Row number
        m_fanTable->setItem(row, 0, new QTableWidgetItem(QString::number(row + 1)));
        
        // Port name
        m_fanTable->setItem(row, 1, new QTableWidgetItem("Port" + QString::number(port)));
        
        // Profile
        m_fanTable->setItem(row, 2, new QTableWidgetItem("Quiet"));
        
        // CPU Temperature (will be updated in real-time)
        m_fanTable->setItem(row, 3, new QTableWidgetItem("CPU 56°C"));
        
        // GPU Load (will be updated in real-time)
        m_fanTable->setItem(row, 4, new QTableWidgetItem("GPU 23%"));
        
        // Fan RPMs (will be updated in real-time)
        m_fanTable->setItem(row, 5, new QTableWidgetItem("975 RPM"));
        
        // Size combo box
        QComboBox *sizeCombo = new QComboBox();
        sizeCombo->addItems({"120mm", "140mm", "200mm"});
        sizeCombo->setCurrentText("120mm");
        m_fanTable->setCellWidget(row, 6, sizeCombo);
        
        row++;
    }
    
    qDebug() << "Updated fan table with" << m_activePorts.size() << "connected ports";
}

bool FanProfilePage::isPortConnected(int port)
{
    if (port < 1 || port > 4) return false;
    return m_portConnected[port - 1];
}
