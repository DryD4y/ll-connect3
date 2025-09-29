#include "fanwidget.h"
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QFont>

FanWidget::FanWidget(int port, int fan, QWidget *parent)
    : QWidget(parent)
    , m_port(port)
    , m_fan(fan)
    , m_rpm(0)
    , m_profile("Quiet")
    , m_lighting("Rainbow")
    , m_fanColor(QColor(42, 130, 218))
    , m_active(true)
{
    setupUI();
    updateAnimation();
}

void FanWidget::setupUI()
{
    setFixedSize(80, 100);
    setObjectName("fanWidget");
    
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(5, 5, 5, 5);
    m_layout->setSpacing(2);
    
    // RPM label
    m_rpmLabel = new QLabel("0 RPM");
    m_rpmLabel->setObjectName("rpmLabel");
    m_rpmLabel->setAlignment(Qt::AlignCenter);
    
    // Lighting label
    m_lightingLabel = new QLabel("Rainbow");
    m_lightingLabel->setObjectName("lightingLabel");
    m_lightingLabel->setAlignment(Qt::AlignCenter);
    
    m_layout->addWidget(m_rpmLabel);
    m_layout->addWidget(m_lightingLabel);
    
    // Initialize color animation
    m_colorAnimation = new QPropertyAnimation(this, "fanColor");
    m_colorAnimation->setDuration(2000);
    m_colorAnimation->setLoopCount(-1); // Infinite loop
    
    // Apply styles
    setStyleSheet(R"(
        #fanWidget {
            background-color: #2d2d2d;
            border: 1px solid #404040;
            border-radius: 8px;
        }
        
        #rpmLabel {
            color: #ffffff;
            font-size: 10px;
            font-weight: bold;
        }
        
        #lightingLabel {
            color: #cccccc;
            font-size: 8px;
        }
    )");
}

void FanWidget::setRPM(int rpm)
{
    m_rpm = rpm;
    m_rpmLabel->setText(QString::number(rpm) + " RPM");
}

void FanWidget::setProfile(const QString &profile)
{
    m_profile = profile;
}

void FanWidget::setLighting(const QString &effect)
{
    m_lighting = effect;
    m_lightingLabel->setText(effect);
    updateAnimation();
}

void FanWidget::setColor(const QColor &color)
{
    m_fanColor = color;
    update();
}

void FanWidget::setActive(bool active)
{
    m_active = active;
    update();
}

void FanWidget::setFanColor(const QColor &color)
{
    m_fanColor = color;
    update();
}

void FanWidget::updateAnimation()
{
    if (m_lighting == "Rainbow" || m_lighting == "Rainbow Morph") {
        // Create rainbow animation
        m_colorAnimation->setKeyValueAt(0, QColor(255, 0, 0));     // Red
        m_colorAnimation->setKeyValueAt(0.16, QColor(255, 165, 0)); // Orange
        m_colorAnimation->setKeyValueAt(0.33, QColor(255, 255, 0)); // Yellow
        m_colorAnimation->setKeyValueAt(0.5, QColor(0, 255, 0));    // Green
        m_colorAnimation->setKeyValueAt(0.66, QColor(0, 0, 255));   // Blue
        m_colorAnimation->setKeyValueAt(0.83, QColor(75, 0, 130));  // Indigo
        m_colorAnimation->setKeyValueAt(1.0, QColor(238, 130, 238)); // Violet
        m_colorAnimation->start();
    } else if (m_lighting == "Static Color") {
        m_colorAnimation->stop();
        setFanColor(QColor(42, 130, 218));
    } else if (m_lighting == "Breathing") {
        m_colorAnimation->stop();
        setFanColor(QColor(42, 130, 218));
    } else {
        m_colorAnimation->stop();
        setFanColor(QColor(42, 130, 218));
    }
}

void FanWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    QRect rect = this->rect().adjusted(10, 10, -10, -10);
    QPoint center = rect.center();
    int radius = qMin(rect.width(), rect.height()) / 2;
    
    // Draw fan blades
    painter.setPen(QPen(m_fanColor, 2));
    painter.setBrush(QBrush(m_fanColor, Qt::SolidPattern));
    
    // Draw 4 fan blades
    for (int i = 0; i < 4; ++i) {
        QPainterPath path;
        double angle = i * 90.0 * M_PI / 180.0;
        double x1 = center.x() + radius * 0.3 * cos(angle);
        double y1 = center.y() + radius * 0.3 * sin(angle);
        double x2 = center.x() + radius * 0.8 * cos(angle);
        double y2 = center.y() + radius * 0.8 * sin(angle);
        
        path.moveTo(x1, y1);
        path.lineTo(x2, y2);
        path.lineTo(x2 + 5 * cos(angle + M_PI/2), y2 + 5 * sin(angle + M_PI/2));
        path.lineTo(x1 + 5 * cos(angle + M_PI/2), y1 + 5 * sin(angle + M_PI/2));
        path.closeSubpath();
        
        painter.drawPath(path);
    }
    
    // Draw center circle
    painter.setPen(QPen(QColor(60, 60, 60), 2));
    painter.setBrush(QBrush(QColor(40, 40, 40)));
    painter.drawEllipse(center, static_cast<int>(radius * 0.2), static_cast<int>(radius * 0.2));
    
    // Draw inactive state
    if (!m_active) {
        painter.setPen(QPen(QColor(100, 100, 100), 1));
        painter.setBrush(QBrush(QColor(100, 100, 100, 100)));
        painter.drawEllipse(rect);
    }
}

void FanWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit clicked();
    }
    QWidget::mousePressEvent(event);
}
