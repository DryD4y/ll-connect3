#include "fanlightingwidget.h"
#include <QPainter>
#include <QTimer>
#include <QDebug>
#include <cmath>

FanLightingWidget::FanLightingWidget(QWidget *parent)
    : QWidget(parent)
    , m_effect("Rainbow")
    , m_speed(50)
    , m_brightness(100)
    , m_directionLeft(false)
    , m_color(Qt::white)
    , m_animationFrame(0)
    , m_timeOffset(0.0)
{
    // Initialize port colors
    m_portColors[0] = QColor(255, 0, 0);   // Port 1 - Red
    m_portColors[1] = QColor(0, 255, 0);   // Port 2 - Green
    m_portColors[2] = QColor(0, 0, 255);   // Port 3 - Blue
    m_portColors[3] = QColor(255, 255, 0); // Port 4 - Yellow
    m_animationTimer = new QTimer(this);
    connect(m_animationTimer, &QTimer::timeout, this, &FanLightingWidget::updateAnimation);
    m_animationTimer->start(50); // 20 FPS
}

void FanLightingWidget::setEffect(const QString &effect)
{
    m_effect = effect;
    update();
}

void FanLightingWidget::setSpeed(int speedPercent)
{
    m_speed = speedPercent;
    update();
}

void FanLightingWidget::setBrightness(int brightnessPercent)
{
    m_brightness = brightnessPercent;
    update();
}

void FanLightingWidget::setDirection(bool leftToRight)
{
    m_directionLeft = leftToRight;
    update();
}

void FanLightingWidget::setColor(const QColor &color)
{
    m_color = color;
    update();
}

void FanLightingWidget::setPortColors(const QColor colors[4])
{
    for (int i = 0; i < 4; ++i) {
        m_portColors[i] = colors[i];
    }
    update();
}

void FanLightingWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Dark background
    painter.fillRect(rect(), QColor(20, 20, 20));
    
    // Draw 6 fans in a 2x3 grid with better spacing
    int fanWidth = (width() - 20) / 2;  // Account for margins
    int fanHeight = (height() - 20) / 3; // Account for margins
    
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 2; ++col) {
            int fanIndex = row * 2 + col;
            QRect fanRect(col * fanWidth + 10, row * fanHeight + 10, fanWidth - 10, fanHeight - 10);
            drawFan(painter, fanRect, fanIndex);
        }
    }
}

void FanLightingWidget::drawFan(QPainter &painter, const QRect &rect, int fanIndex)
{
    // Draw fan frame
    painter.setPen(QPen(QColor(60, 60, 60), 2));
    painter.setBrush(QColor(40, 40, 40));
    painter.drawRoundedRect(rect.adjusted(5, 5, -5, -5), 8, 8);
    
    // Draw fan blades (simplified)
    QRect fanArea = rect.adjusted(15, 15, -15, -15);
    int centerX = fanArea.center().x();
    int centerY = fanArea.center().y();
    int radius = qMin(fanArea.width(), fanArea.height()) / 2 - 10;
    
    // Draw 8 fan blades
    for (int i = 0; i < 8; ++i) {
        double angle = (i * 45.0) * M_PI / 180.0;
        int x1 = centerX + cos(angle) * (radius - 15);
        int y1 = centerY + sin(angle) * (radius - 15);
        int x2 = centerX + cos(angle) * radius;
        int y2 = centerY + sin(angle) * radius;
        
        painter.setPen(QPen(QColor(80, 80, 80), 2));
        painter.drawLine(x1, y1, x2, y2);
    }
    
    // Draw LED ring around the fan
    QRect ledRing = fanArea.adjusted(-5, -5, 5, 5);
    
    if (m_effect == "Rainbow") {
        drawRainbowEffect(painter, ledRing, fanIndex);
    } else if (m_effect == "Rainbow Morph") {
        drawRainbowMorphEffect(painter, ledRing, fanIndex);
    } else if (m_effect == "Static Color") {
        drawStaticColorEffect(painter, ledRing, fanIndex);
    } else if (m_effect == "Breathing") {
        drawBreathingEffect(painter, ledRing, fanIndex);
    } else if (m_effect == "Meteor") {
        drawMeteorEffect(painter, ledRing, fanIndex);
    } else if (m_effect == "Runway") {
        drawRunwayEffect(painter, ledRing, fanIndex);
    }
}

void FanLightingWidget::drawRainbowEffect(QPainter &painter, const QRect &rect, int fanIndex)
{
    int centerX = rect.center().x();
    int centerY = rect.center().y();
    int radius = qMin(rect.width(), rect.height()) / 2;
    
    // Calculate animation offset based on speed and direction
    double speedMultiplier = m_speed / 100.0;
    double timeOffset = m_timeOffset * speedMultiplier;
    if (m_directionLeft) timeOffset = -timeOffset;
    
    // Draw 16 LED segments around the ring
    for (int i = 0; i < 16; ++i) {
        double angle = (i * 22.5) * M_PI / 180.0 + timeOffset;
        QColor color = getRainbowColor(i + fanIndex * 2, 16);
        color = applyBrightness(color, m_brightness);
        
        painter.setPen(QPen(color, 4));
        int x1 = centerX + cos(angle) * (radius - 8);
        int y1 = centerY + sin(angle) * (radius - 8);
        int x2 = centerX + cos(angle) * radius;
        int y2 = centerY + sin(angle) * radius;
        
        painter.drawLine(x1, y1, x2, y2);
    }
}

void FanLightingWidget::drawRainbowMorphEffect(QPainter &painter, const QRect &rect, int fanIndex)
{
    int centerX = rect.center().x();
    int centerY = rect.center().y();
    int radius = qMin(rect.width(), rect.height()) / 2;
    
    // Calculate animation offset
    double speedMultiplier = m_speed / 100.0;
    double timeOffset = m_timeOffset * speedMultiplier * 0.5; // Slower than rainbow
    if (m_directionLeft) timeOffset = -timeOffset;
    
    // Draw morphing rainbow pattern
    for (int i = 0; i < 16; ++i) {
        double angle = (i * 22.5) * M_PI / 180.0;
        double morphOffset = sin(timeOffset + i * 0.5) * 0.3;
        int colorPosition = i + fanIndex * 3 + static_cast<int>(morphOffset * 16);
        QColor color = getRainbowColor(colorPosition, 16);
        color = applyBrightness(color, m_brightness);
        
        painter.setPen(QPen(color, 4));
        int x1 = centerX + cos(angle) * (radius - 8);
        int y1 = centerY + sin(angle) * (radius - 8);
        int x2 = centerX + cos(angle) * radius;
        int y2 = centerY + sin(angle) * radius;
        
        painter.drawLine(x1, y1, x2, y2);
    }
}

void FanLightingWidget::drawStaticColorEffect(QPainter &painter, const QRect &rect, int fanIndex)
{
    int centerX = rect.center().x();
    int centerY = rect.center().y();
    int radius = qMin(rect.width(), rect.height()) / 2;
    
    // Use port-specific color (fanIndex 0-3 maps to ports 1-4)
    QColor portColor = m_portColors[fanIndex % 4];
    QColor color = applyBrightness(portColor, m_brightness);
    painter.setPen(QPen(color, 6));
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(centerX - radius + 5, centerY - radius + 5, (radius - 5) * 2, (radius - 5) * 2);
}

void FanLightingWidget::drawBreathingEffect(QPainter &painter, const QRect &rect, int fanIndex)
{
    Q_UNUSED(fanIndex)
    
    int centerX = rect.center().x();
    int centerY = rect.center().y();
    int radius = qMin(rect.width(), rect.height()) / 2;
    
    // Calculate breathing animation
    double speedMultiplier = m_speed / 100.0;
    double breathPhase = sin(m_timeOffset * speedMultiplier * 2.0) * 0.5 + 0.5;
    int brightness = m_brightness * (0.3 + 0.7 * breathPhase);
    
    QColor color = applyBrightness(m_color, brightness);
    painter.setPen(QPen(color, 6));
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(centerX - radius + 5, centerY - radius + 5, (radius - 5) * 2, (radius - 5) * 2);
}

void FanLightingWidget::drawMeteorEffect(QPainter &painter, const QRect &rect, int fanIndex)
{
    int centerX = rect.center().x();
    int centerY = rect.center().y();
    int radius = qMin(rect.width(), rect.height()) / 2;
    
    // Calculate meteor position (faster movement for meteor effect)
    double speedMultiplier = m_speed / 100.0;
    double meteorPos = fmod(m_timeOffset * speedMultiplier * 1.5, 1.0);
    if (m_directionLeft) meteorPos = 1.0 - meteorPos;
    
    // Draw meteor with bright head and fading trail
    for (int i = 0; i < 16; ++i) {
        double angle = (i * 22.5) * M_PI / 180.0;
        double ledPosition = fmod(meteorPos * 16 + i, 16) / 16.0;
        
        // Create meteor trail (shorter and more focused)
        if (ledPosition < 0.4) { // Show meteor trail for 40% of the ring
            double trailIntensity = 1.0 - (ledPosition / 0.4); // Fade from 1.0 to 0.0
            trailIntensity = trailIntensity * trailIntensity; // Square for more dramatic fade
            
            // Bright head, fading tail
            int intensity = static_cast<int>(m_brightness * trailIntensity);
            QColor color = applyBrightness(m_color, intensity);
            
            // Make the head brighter and larger
            int penWidth = (ledPosition < 0.1) ? 6 : 4; // Thicker pen for the head
            
            painter.setPen(QPen(color, penWidth));
            
            int x1 = centerX + cos(angle) * (radius - 8);
            int y1 = centerY + sin(angle) * (radius - 8);
            int x2 = centerX + cos(angle) * radius;
            int y2 = centerY + sin(angle) * radius;
            
            painter.drawLine(x1, y1, x2, y2);
        }
    }
}

void FanLightingWidget::drawRunwayEffect(QPainter &painter, const QRect &rect, int fanIndex)
{
    int centerX = rect.center().x();
    int centerY = rect.center().y();
    int radius = qMin(rect.width(), rect.height()) / 2;
    
    // Calculate runway animation
    double speedMultiplier = m_speed / 100.0;
    double runwayPos = fmod(m_timeOffset * speedMultiplier * 1.2, 1.0);
    if (m_directionLeft) runwayPos = 1.0 - runwayPos;
    
    // Draw runway lights around the circular LED ring
    for (int i = 0; i < 16; ++i) {
        double angle = (i * 22.5) * M_PI / 180.0;
        double distance = fmod(runwayPos * 16 + i, 16) / 16.0;
        
        if (distance < 0.2) { // Short runway segment
            QColor color = applyBrightness(m_color, m_brightness);
            painter.setPen(QPen(color, 4));
            
            int x1 = centerX + cos(angle) * (radius - 8);
            int y1 = centerY + sin(angle) * (radius - 8);
            int x2 = centerX + cos(angle) * radius;
            int y2 = centerY + sin(angle) * radius;
            
            painter.drawLine(x1, y1, x2, y2);
        }
    }
}

QColor FanLightingWidget::getRainbowColor(int position, int totalPositions) const
{
    // Ensure position is within valid range
    position = position % totalPositions;
    if (position < 0) position += totalPositions;
    
    // Calculate hue (0-359 degrees)
    double hue = (position * 360.0) / totalPositions;
    hue = fmod(hue, 360.0);
    if (hue < 0) hue += 360.0;
    
    return QColor::fromHsv(static_cast<int>(hue), 255, 255);
}

QColor FanLightingWidget::applyBrightness(const QColor &color, int brightnessPercent) const
{
    double factor = brightnessPercent / 100.0;
    return QColor(
        color.red() * factor,
        color.green() * factor,
        color.blue() * factor
    );
}

void FanLightingWidget::updateAnimation()
{
    m_timeOffset += 0.1;
    m_animationFrame++;
    update();
}
