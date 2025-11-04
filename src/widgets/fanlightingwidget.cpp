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
    
    // Initialize all ports as enabled by default
    m_portEnabled[0] = true;
    m_portEnabled[1] = true;
    m_portEnabled[2] = true;
    m_portEnabled[3] = true;
    
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

void FanLightingWidget::setPortEnabled(const bool enabled[4])
{
    for (int i = 0; i < 4; ++i) {
        m_portEnabled[i] = enabled[i];
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
    
    // Draw 4 fans in a 2x2 grid (matching 4 physical ports)
    int fanWidth = (width() - 20) / 2;  // Account for margins
    int fanHeight = (height() - 20) / 2; // Account for margins (2 rows for 4 fans)
    
    for (int row = 0; row < 2; ++row) {
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
    
    // Check if this port is enabled
    bool portEnabled = m_portEnabled[fanIndex % 4];
    
    if (!portEnabled) {
        // Draw disabled/grayed out LED ring
        int ledCenterX = ledRing.center().x();
        int ledCenterY = ledRing.center().y();
        int ledRadius = qMin(ledRing.width(), ledRing.height()) / 2;
        painter.setPen(QPen(QColor(60, 60, 60), 6));
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(ledCenterX - ledRadius + 5, ledCenterY - ledRadius + 5, (ledRadius - 5) * 2, (ledRadius - 5) * 2);
    } else {
        // Draw normal lighting effect - handle all effects with proper name mapping
        if (m_effect == "Rainbow Wave" || m_effect == "Rainbow") {
            drawRainbowEffect(painter, ledRing, fanIndex);
        } else if (m_effect == "Spectrum Cycle" || m_effect == "Rainbow Morph") {
            drawRainbowMorphEffect(painter, ledRing, fanIndex);
        } else if (m_effect == "Static" || m_effect == "Static Color") {
            drawStaticColorEffect(painter, ledRing, fanIndex);
        } else if (m_effect == "Breathing") {
            drawBreathingEffect(painter, ledRing, fanIndex);
        } else if (m_effect == "Meteor") {
            drawMeteorEffect(painter, ledRing, fanIndex);
        } else if (m_effect == "Runway") {
            drawRunwayEffect(painter, ledRing, fanIndex);
        } else if (m_effect == "Groove") {
            drawGrooveEffect(painter, ledRing, fanIndex);
        } else if (m_effect == "Mixing") {
            drawMixingEffect(painter, ledRing, fanIndex);
        } else if (m_effect == "Neon") {
            drawNeonEffect(painter, ledRing, fanIndex);
        } else if (m_effect == "Stack") {
            drawStackEffect(painter, ledRing, fanIndex);
        } else if (m_effect == "Staggered") {
            drawStaggeredEffect(painter, ledRing, fanIndex);
        } else if (m_effect == "Tide") {
            drawTideEffect(painter, ledRing, fanIndex);
        } else if (m_effect == "Tunnel") {
            drawTunnelEffect(painter, ledRing, fanIndex);
        } else if (m_effect == "Voice") {
            drawVoiceEffect(painter, ledRing, fanIndex);
        } else {
            // Default fallback - draw static white
            drawStaticColorEffect(painter, ledRing, fanIndex);
        }
    }
    
    // Draw port label at the bottom of the fan
    painter.setPen(QColor(200, 200, 200)); // Light gray text
    painter.setFont(QFont("Arial", 10, QFont::Normal));
    QString label = QString("Port %1").arg(fanIndex + 1);
    QRect labelRect(rect.left(), rect.bottom() - 25, rect.width(), 20);
    painter.drawText(labelRect, Qt::AlignCenter, label);
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
    int centerX = rect.center().x();
    int centerY = rect.center().y();
    int radius = qMin(rect.width(), rect.height()) / 2;
    
    // Calculate breathing animation
    double speedMultiplier = m_speed / 100.0;
    double breathPhase = sin(m_timeOffset * speedMultiplier * 2.0) * 0.5 + 0.5;
    int brightness = m_brightness * (0.3 + 0.7 * breathPhase);
    
    // Use port-specific color (fanIndex 0-3 maps to ports 1-4)
    QColor portColor = m_portColors[fanIndex % 4];
    QColor color = applyBrightness(portColor, brightness);
    painter.setPen(QPen(color, 6));
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(centerX - radius + 5, centerY - radius + 5, (radius - 5) * 2, (radius - 5) * 2);
}

void FanLightingWidget::drawMeteorEffect(QPainter &painter, const QRect &rect, int fanIndex)
{
    int centerX = rect.center().x();
    int centerY = rect.center().y();
    int radius = qMin(rect.width(), rect.height()) / 2;
    
    // Use port-specific color (fanIndex 0-3 maps to ports 1-4)
    QColor portColor = m_portColors[fanIndex % 4];
    
    // Calculate meteor position (faster movement for meteor effect)
    // The meteor should travel around the circle continuously
    double speedMultiplier = m_speed / 100.0;
    double meteorPos = fmod(m_timeOffset * speedMultiplier * 1.5, 1.0);
    if (m_directionLeft) meteorPos = 1.0 - meteorPos;
    
    // Draw meteor with bright head and fading trail traveling around the circle
    // meteorPos is 0.0 to 1.0, representing position around the circle
    int meteorHeadLED = static_cast<int>(meteorPos * 16) % 16;
    
    for (int i = 0; i < 16; ++i) {
        // Calculate distance from this LED to the meteor head (wrapping around)
        int distance = (i - meteorHeadLED + 16) % 16;
        double ledPosition = distance / 16.0;
        
        // Create meteor trail (shorter and more focused - about 5-6 LEDs)
        if (distance <= 5) {
            double trailIntensity = 1.0 - (ledPosition / (5.0 / 16.0)); // Fade from 1.0 to 0.0
            if (trailIntensity < 0) trailIntensity = 0;
            trailIntensity = trailIntensity * trailIntensity; // Square for more dramatic fade
            
            // Bright head, fading tail
            int intensity = static_cast<int>(m_brightness * trailIntensity);
            QColor color = applyBrightness(portColor, intensity);
            
            // Calculate the angle for this LED position
            double angle = (i * 22.5) * M_PI / 180.0;
            
            // Make the head brighter and larger
            int penWidth = (distance == 0) ? 6 : 4; // Thicker pen for the head
            
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

void FanLightingWidget::drawGrooveEffect(QPainter &painter, const QRect &rect, int fanIndex)
{
    int centerX = rect.center().x();
    int centerY = rect.center().y();
    int radius = qMin(rect.width(), rect.height()) / 2;
    
    // Use port-specific color
    QColor portColor = m_portColors[fanIndex % 4];
    
    // Groove: Rotating segments with color
    double speedMultiplier = m_speed / 100.0;
    double timeOffset = m_timeOffset * speedMultiplier;
    if (m_directionLeft) timeOffset = -timeOffset;
    
    // Draw 4 segments rotating around the circle
    for (int i = 0; i < 4; ++i) {
        double angle = (i * 90.0) * M_PI / 180.0 + timeOffset;
        double segmentLength = 22.5; // 22.5 degrees per segment
        
        for (int j = 0; j < 4; ++j) {
            double ledAngle = angle + (j * 22.5) * M_PI / 180.0;
            QColor color = applyBrightness(portColor, m_brightness);
            painter.setPen(QPen(color, 4));
            
            int x1 = centerX + cos(ledAngle) * (radius - 8);
            int y1 = centerY + sin(ledAngle) * (radius - 8);
            int x2 = centerX + cos(ledAngle) * radius;
            int y2 = centerY + sin(ledAngle) * radius;
            
            painter.drawLine(x1, y1, x2, y2);
        }
    }
}

void FanLightingWidget::drawMixingEffect(QPainter &painter, const QRect &rect, int fanIndex)
{
    int centerX = rect.center().x();
    int centerY = rect.center().y();
    int radius = qMin(rect.width(), rect.height()) / 2;
    
    // Mixing: One color per port, mixing with itself in a wave pattern
    QColor portColor = m_portColors[fanIndex % 4];
    
    // Create a lighter and darker version for the mixing effect
    QColor color1 = portColor;
    QColor color2 = QColor(
        qBound(0, portColor.red() + 50, 255),
        qBound(0, portColor.green() + 50, 255),
        qBound(0, portColor.blue() + 50, 255)
    );
    
    double speedMultiplier = m_speed / 100.0;
    double timeOffset = m_timeOffset * speedMultiplier;
    
    for (int i = 0; i < 16; ++i) {
        double angle = (i * 22.5) * M_PI / 180.0;
        double mixPhase = sin(timeOffset + i * 0.5) * 0.5 + 0.5; // 0 to 1
        
        // Blend between color1 and color2
        QColor color(
            color1.red() * (1.0 - mixPhase) + color2.red() * mixPhase,
            color1.green() * (1.0 - mixPhase) + color2.green() * mixPhase,
            color1.blue() * (1.0 - mixPhase) + color2.blue() * mixPhase
        );
        color = applyBrightness(color, m_brightness);
        
        painter.setPen(QPen(color, 4));
        int x1 = centerX + cos(angle) * (radius - 8);
        int y1 = centerY + sin(angle) * (radius - 8);
        int x2 = centerX + cos(angle) * radius;
        int y2 = centerY + sin(angle) * radius;
        
        painter.drawLine(x1, y1, x2, y2);
    }
}

void FanLightingWidget::drawNeonEffect(QPainter &painter, const QRect &rect, int fanIndex)
{
    int centerX = rect.center().x();
    int centerY = rect.center().y();
    int radius = qMin(rect.width(), rect.height()) / 2;
    
    // Neon: Pulsing glow effect with port-specific color
    QColor portColor = m_portColors[fanIndex % 4];
    
    double speedMultiplier = m_speed / 100.0;
    // Create a pulsing neon glow effect - smooth pulse, not wave
    double pulsePhase = sin(m_timeOffset * speedMultiplier * 2.0) * 0.5 + 0.5; // 0 to 1
    int pulseBrightness = m_brightness * (0.4 + 0.6 * pulsePhase); // Pulse between 40% and 100% brightness
    
    // Draw uniform neon glow around the entire ring (not a wave pattern)
    for (int i = 0; i < 16; ++i) {
        double angle = (i * 22.5) * M_PI / 180.0;
        QColor color = applyBrightness(portColor, pulseBrightness);
        
        // Make it look more neon-like with a glow effect (thicker pen)
        painter.setPen(QPen(color, 6));
        int x1 = centerX + cos(angle) * (radius - 8);
        int y1 = centerY + sin(angle) * (radius - 8);
        int x2 = centerX + cos(angle) * radius;
        int y2 = centerY + sin(angle) * radius;
        
        painter.drawLine(x1, y1, x2, y2);
    }
}

void FanLightingWidget::drawStackEffect(QPainter &painter, const QRect &rect, int fanIndex)
{
    int centerX = rect.center().x();
    int centerY = rect.center().y();
    int radius = qMin(rect.width(), rect.height()) / 2;
    
    // Stack: Lights stacking up from bottom
    QColor portColor = m_portColors[fanIndex % 4];
    
    double speedMultiplier = m_speed / 100.0;
    double stackPos = fmod(m_timeOffset * speedMultiplier, 1.0);
    if (m_directionLeft) stackPos = 1.0 - stackPos;
    
    // Draw stacked segments
    for (int i = 0; i < 16; ++i) {
        double angle = (i * 22.5) * M_PI / 180.0;
        double normalizedPos = (i / 16.0);
        
        if (normalizedPos <= stackPos) {
            QColor color = applyBrightness(portColor, m_brightness);
            painter.setPen(QPen(color, 4));
            
            int x1 = centerX + cos(angle) * (radius - 8);
            int y1 = centerY + sin(angle) * (radius - 8);
            int x2 = centerX + cos(angle) * radius;
            int y2 = centerY + sin(angle) * radius;
            
            painter.drawLine(x1, y1, x2, y2);
        }
    }
}

void FanLightingWidget::drawStaggeredEffect(QPainter &painter, const QRect &rect, int fanIndex)
{
    int centerX = rect.center().x();
    int centerY = rect.center().y();
    int radius = qMin(rect.width(), rect.height()) / 2;
    
    // Staggered: Alternating colors in a staggered pattern
    QColor color1 = m_portColors[fanIndex % 4];
    QColor color2 = (fanIndex % 4 < 3) ? m_portColors[(fanIndex % 4) + 1] : m_portColors[0];
    
    double speedMultiplier = m_speed / 100.0;
    double timeOffset = m_timeOffset * speedMultiplier;
    
    for (int i = 0; i < 16; ++i) {
        double angle = (i * 22.5) * M_PI / 180.0;
        int pattern = (static_cast<int>(timeOffset * 4) + i) % 4;
        
        QColor color = (pattern < 2) ? color1 : color2;
        color = applyBrightness(color, m_brightness);
        painter.setPen(QPen(color, 4));
        
        int x1 = centerX + cos(angle) * (radius - 8);
        int y1 = centerY + sin(angle) * (radius - 8);
        int x2 = centerX + cos(angle) * radius;
        int y2 = centerY + sin(angle) * radius;
        
        painter.drawLine(x1, y1, x2, y2);
    }
}

void FanLightingWidget::drawTideEffect(QPainter &painter, const QRect &rect, int fanIndex)
{
    int centerX = rect.center().x();
    int centerY = rect.center().y();
    int radius = qMin(rect.width(), rect.height()) / 2;
    
    // Tide: Wave-like effect with two colors
    QColor color1 = m_portColors[fanIndex % 4];
    QColor color2 = (fanIndex % 4 < 3) ? m_portColors[(fanIndex % 4) + 1] : m_portColors[0];
    
    double speedMultiplier = m_speed / 100.0;
    double timeOffset = m_timeOffset * speedMultiplier;
    
    for (int i = 0; i < 16; ++i) {
        double angle = (i * 22.5) * M_PI / 180.0;
        double wavePhase = sin(timeOffset * 2.0 + i * 0.5) * 0.5 + 0.5; // 0 to 1
        
        // Blend between color1 and color2 based on wave
        QColor color(
            color1.red() * (1.0 - wavePhase) + color2.red() * wavePhase,
            color1.green() * (1.0 - wavePhase) + color2.green() * wavePhase,
            color1.blue() * (1.0 - wavePhase) + color2.blue() * wavePhase
        );
        color = applyBrightness(color, m_brightness);
        
        painter.setPen(QPen(color, 4));
        int x1 = centerX + cos(angle) * (radius - 8);
        int y1 = centerY + sin(angle) * (radius - 8);
        int x2 = centerX + cos(angle) * radius;
        int y2 = centerY + sin(angle) * radius;
        
        painter.drawLine(x1, y1, x2, y2);
    }
}

void FanLightingWidget::drawTunnelEffect(QPainter &painter, const QRect &rect, int fanIndex)
{
    int centerX = rect.center().x();
    int centerY = rect.center().y();
    int radius = qMin(rect.width(), rect.height()) / 2;
    
    // Tunnel: Spiral effect with multiple colors
    QColor portColor = m_portColors[fanIndex % 4];
    
    double speedMultiplier = m_speed / 100.0;
    double timeOffset = m_timeOffset * speedMultiplier;
    if (m_directionLeft) timeOffset = -timeOffset;
    
    // Draw tunnel/spiral pattern
    for (int i = 0; i < 16; ++i) {
        double angle = (i * 22.5) * M_PI / 180.0;
        double spiralOffset = (i + timeOffset * 16) * 0.1;
        double brightnessMod = 0.3 + 0.7 * (sin(spiralOffset) * 0.5 + 0.5);
        
        QColor color = applyBrightness(portColor, m_brightness * brightnessMod);
        painter.setPen(QPen(color, 4));
        
        int x1 = centerX + cos(angle) * (radius - 8);
        int y1 = centerY + sin(angle) * (radius - 8);
        int x2 = centerX + cos(angle) * radius;
        int y2 = centerY + sin(angle) * radius;
        
        painter.drawLine(x1, y1, x2, y2);
    }
}

void FanLightingWidget::drawVoiceEffect(QPainter &painter, const QRect &rect, int fanIndex)
{
    int centerX = rect.center().x();
    int centerY = rect.center().y();
    int radius = qMin(rect.width(), rect.height()) / 2;
    
    // Voice: Sound-reactive pulsing effect
    double speedMultiplier = m_speed / 100.0;
    double pulsePhase = sin(m_timeOffset * speedMultiplier * 3.0) * 0.5 + 0.5;
    double pulse2Phase = sin(m_timeOffset * speedMultiplier * 4.0 + 1.0) * 0.5 + 0.5;
    
    // Pulsing rainbow colors
    for (int i = 0; i < 16; ++i) {
        double angle = (i * 22.5) * M_PI / 180.0;
        double pulseIntensity = (i % 2 == 0) ? pulsePhase : pulse2Phase;
        int brightness = m_brightness * (0.4 + 0.6 * pulseIntensity);
        
        QColor color = getRainbowColor(i + fanIndex * 2, 16);
        color = applyBrightness(color, brightness);
        painter.setPen(QPen(color, 4));
        
        int x1 = centerX + cos(angle) * (radius - 8);
        int y1 = centerY + sin(angle) * (radius - 8);
        int x2 = centerX + cos(angle) * radius;
        int y2 = centerY + sin(angle) * radius;
        
        painter.drawLine(x1, y1, x2, y2);
    }
}
