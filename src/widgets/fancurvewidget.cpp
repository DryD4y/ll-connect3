#include "fancurvewidget.h"
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QFont>
#include <QFontMetrics>
#include <cmath>

FanCurveWidget::FanCurveWidget(QWidget *parent)
    : QWidget(parent)
    , m_profile("Quiet")
    , m_currentTemperature(25)
    , m_currentRPM(420)
    , m_marginLeft(50)
    , m_marginRight(20)
    , m_marginTop(20)
    , m_marginBottom(40)
    , m_tempMin(0)
    , m_tempMax(100)
    , m_rpmMin(0)
    , m_rpmMax(2100)
    , m_dragging(false)
    , m_draggedPoint(-1)
    , m_backgroundColor(QColor(26, 26, 26))
    , m_gridColor(QColor(60, 60, 60))
    , m_axisColor(QColor(200, 200, 200))
    , m_curveColor(QColor(100, 150, 255))
    , m_pointColor(QColor(255, 255, 255))
    , m_currentLineColor(QColor(0, 255, 0))
{
    setMinimumSize(400, 200);
    setupCurveData();
}

void FanCurveWidget::setProfile(const QString &profile)
{
    m_profile = profile;
    setupCurveData();
    update();
}

void FanCurveWidget::setCurrentTemperature(int temperature)
{
    m_currentTemperature = temperature;
    update();
}

void FanCurveWidget::setCurrentRPM(int rpm)
{
    m_currentRPM = rpm;
    update();
}

void FanCurveWidget::setupCurveData()
{
    m_curvePoints.clear();
    
    if (m_profile == "Quiet") {
        // Data points for 120mm fans (max 2100 RPM)
        m_curvePoints << QPointF(0, 0);
        m_curvePoints << QPointF(25, 420);
        m_curvePoints << QPointF(45, 840);
        m_curvePoints << QPointF(65, 1050);
        m_curvePoints << QPointF(80, 1680);
        m_curvePoints << QPointF(90, 2100);
        m_curvePoints << QPointF(100, 2100);
    } else if (m_profile == "Standard") {
        // More aggressive curve for Standard
        m_curvePoints << QPointF(0, 200);
        m_curvePoints << QPointF(20, 500);
        m_curvePoints << QPointF(40, 1000);
        m_curvePoints << QPointF(60, 1400);
        m_curvePoints << QPointF(75, 1800);
        m_curvePoints << QPointF(85, 2100);
    } else if (m_profile == "High Speed") {
        // Very aggressive curve
        m_curvePoints << QPointF(0, 300);
        m_curvePoints << QPointF(15, 800);
        m_curvePoints << QPointF(35, 1300);
        m_curvePoints << QPointF(55, 1800);
        m_curvePoints << QPointF(70, 2100);
        m_curvePoints << QPointF(80, 2100);
    } else if (m_profile == "Full Speed") {
        // Maximum speed curve
        m_curvePoints << QPointF(0, 800);
        m_curvePoints << QPointF(10, 1200);
        m_curvePoints << QPointF(30, 1800);
        m_curvePoints << QPointF(50, 2100);
        m_curvePoints << QPointF(70, 2100);
        m_curvePoints << QPointF(90, 2100);
    } else if (m_profile == "MB RPM Sync") {
        // Custom curve for MB RPM Sync
        m_curvePoints << QPointF(0, 100);
        m_curvePoints << QPointF(30, 400);
        m_curvePoints << QPointF(50, 800);
        m_curvePoints << QPointF(70, 1400);
        m_curvePoints << QPointF(85, 1900);
        m_curvePoints << QPointF(95, 2100);
    } else {
        // Default to Quiet (original Lian Li curve)
        m_curvePoints << QPointF(0, 0);
        m_curvePoints << QPointF(25, 420);
        m_curvePoints << QPointF(45, 840);
        m_curvePoints << QPointF(65, 1050);
        m_curvePoints << QPointF(80, 1680);
        m_curvePoints << QPointF(90, 2100);
        m_curvePoints << QPointF(100, 2100);
    }
}

void FanCurveWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Fill background
    painter.fillRect(rect(), m_backgroundColor);
    
    // Calculate graph area
    QRect graphRect = rect().adjusted(m_marginLeft, m_marginTop, -m_marginRight, -m_marginBottom);
    
    // Draw grid
    drawGrid(painter);
    
    // Draw axes
    drawAxes(painter);
    
    // Draw curve
    drawCurve(painter);
    
    // Draw data points
    drawDataPoints(painter);
    
    // Draw current temperature line
    drawCurrentLine(painter);
}

void FanCurveWidget::drawGrid(QPainter &painter)
{
    QRect graphRect = rect().adjusted(m_marginLeft, m_marginTop, -m_marginRight, -m_marginBottom);
    
    painter.setPen(QPen(m_gridColor, 1));
    
    // Vertical grid lines (temperature)
    for (int temp = 0; temp <= 100; temp += 10) {
        int x = graphRect.left() + (temp - m_tempMin) / (m_tempMax - m_tempMin) * graphRect.width();
        painter.drawLine(x, graphRect.top(), x, graphRect.bottom());
    }
    
    // Minor vertical grid lines (every 5°C)
    painter.setPen(QPen(m_gridColor, 0.5));
    for (int temp = 5; temp < 100; temp += 10) {
        int x = graphRect.left() + (temp - m_tempMin) / (m_tempMax - m_tempMin) * graphRect.width();
        painter.drawLine(x, graphRect.top(), x, graphRect.bottom());
    }
    
    // Horizontal grid lines (RPM)
    painter.setPen(QPen(m_gridColor, 1));
    for (int rpm = 0; rpm <= 2100; rpm += 420) {
        int y = graphRect.bottom() - (rpm - m_rpmMin) / (m_rpmMax - m_rpmMin) * graphRect.height();
        painter.drawLine(graphRect.left(), y, graphRect.right(), y);
    }
    
    // Minor horizontal grid lines
    painter.setPen(QPen(m_gridColor, 0.5));
    for (int rpm = 210; rpm < 2100; rpm += 420) {
        int y = graphRect.bottom() - (rpm - m_rpmMin) / (m_rpmMax - m_rpmMin) * graphRect.height();
        painter.drawLine(graphRect.left(), y, graphRect.right(), y);
    }
}

void FanCurveWidget::drawAxes(QPainter &painter)
{
    QRect graphRect = rect().adjusted(m_marginLeft, m_marginTop, -m_marginRight, -m_marginBottom);
    
    painter.setPen(QPen(m_axisColor, 2));
    
    // Draw axes
    painter.drawLine(graphRect.left(), graphRect.bottom(), graphRect.right(), graphRect.bottom()); // X-axis
    painter.drawLine(graphRect.left(), graphRect.top(), graphRect.left(), graphRect.bottom()); // Y-axis
    
    // Draw axis labels
    painter.setPen(QPen(m_axisColor, 1));
    QFont font = painter.font();
    font.setPointSize(9);
    painter.setFont(font);
    
    // X-axis labels (Temperature)
    for (int temp = 0; temp <= 100; temp += 10) {
        int x = graphRect.left() + (temp - m_tempMin) / (m_tempMax - m_tempMin) * graphRect.width();
        painter.drawText(x - 15, graphRect.bottom() + 15, 30, 15, Qt::AlignCenter, QString::number(temp) + "°C");
    }
    
    // Y-axis labels (RPM)
    for (int rpm = 0; rpm <= 2100; rpm += 420) {
        int y = graphRect.bottom() - (rpm - m_rpmMin) / (m_rpmMax - m_rpmMin) * graphRect.height();
        painter.drawText(5, y - 10, m_marginLeft - 10, 20, Qt::AlignRight | Qt::AlignVCenter, QString::number(rpm));
    }
}

void FanCurveWidget::drawCurve(QPainter &painter)
{
    if (m_curvePoints.size() < 2) return;
    
    QRect graphRect = rect().adjusted(m_marginLeft, m_marginTop, -m_marginRight, -m_marginBottom);
    
    painter.setPen(QPen(m_curveColor, 2));
    
    // Draw curve line
    QPainterPath path;
    QPointF firstPoint = dataToPixel(m_curvePoints[0]);
    path.moveTo(firstPoint);
    
    for (int i = 1; i < m_curvePoints.size(); ++i) {
        QPointF point = dataToPixel(m_curvePoints[i]);
        path.lineTo(point);
    }
    
    painter.drawPath(path);
}

void FanCurveWidget::drawDataPoints(QPainter &painter)
{
    QRect graphRect = rect().adjusted(m_marginLeft, m_marginTop, -m_marginRight, -m_marginBottom);
    
    for (int i = 0; i < m_curvePoints.size(); ++i) {
        QPointF point = dataToPixel(m_curvePoints[i]);
        
        // Draw white circle with blue outline
        painter.setPen(QPen(m_curveColor, 2));
        painter.setBrush(QBrush(m_pointColor));
        painter.drawEllipse(point, 4, 4);
    }
}

QColor FanCurveWidget::getTemperatureColor(int temperature)
{
    if (temperature <= 41) {
        // 0-41°C: Blue (cool)
        return QColor(0, 150, 255);
    } else if (temperature <= 60) {
        // 42-60°C: Green (normal)
        return QColor(0, 255, 0);
    } else if (temperature <= 76) {
        // 61-76°C: Yellow (warm)
        return QColor(255, 255, 0);
    } else {
        // 77-100°C: Red (hot)
        return QColor(255, 0, 0);
    }
}

void FanCurveWidget::drawCurrentLine(QPainter &painter)
{
    QRect graphRect = rect().adjusted(m_marginLeft, m_marginTop, -m_marginRight, -m_marginBottom);
    
    // Calculate color based on temperature
    QColor lineColor = getTemperatureColor(m_currentTemperature);
    
    // Draw vertical line at current temperature
    int x = graphRect.left() + (m_currentTemperature - m_tempMin) / (m_tempMax - m_tempMin) * graphRect.width();
    
    // Draw a thicker, more prominent line
    painter.setPen(QPen(lineColor, 3));
    painter.drawLine(x, graphRect.top(), x, graphRect.bottom());
    
    // Calculate the RPM that should be on the curve at this temperature
    int curveRPM = calculateRPMForTemperature(m_currentTemperature);
    
    // Draw a larger circle at the intersection with the curve (temperature ball)
    QPointF curvePoint = QPointF(x, graphRect.bottom() - (curveRPM - m_rpmMin) / (m_rpmMax - m_rpmMin) * graphRect.height());
    painter.setPen(QPen(lineColor, 2));
    painter.setBrush(QBrush(lineColor));
    painter.drawEllipse(curvePoint, 7, 7); // Increased from 6 to 7 (about 17% larger)
    
    // Draw a fixed orange center dot (not moving)
    QColor rpmColor = QColor(255, 165, 0); // Orange color
    painter.setPen(QPen(rpmColor, 2));
    painter.setBrush(QBrush(rpmColor));
    painter.drawEllipse(curvePoint, 4, 4); // Fixed at the same position as temperature ball
    
    // Temperature label removed - shown in table above instead
    
    // Draw legend inside the graph area on the left side
    painter.setPen(QPen(Qt::white, 1));
    QFont legendFont = painter.font();
    legendFont.setPointSize(7);
    legendFont.setBold(false);
    painter.setFont(legendFont);
    
    // Position legend inside the graph area, around 0-15°C and 1000-1500 RPM range
    int legendX = graphRect.left() + 20;  // Inside the graph, 20px from left edge
    int legendY = graphRect.top() + 60;   // Moved up by 20px from previous position
    
    // Temperature ball legend
    painter.setPen(QPen(lineColor, 2));
    painter.setBrush(QBrush(lineColor));
    painter.drawEllipse(legendX, legendY, 6, 6);
    painter.setPen(QPen(Qt::white, 1));
    painter.drawText(legendX + 10, legendY + 4, 80, 15, Qt::AlignLeft | Qt::AlignVCenter, "Temp Target");
    
    // Fixed orange dot legend
    painter.setPen(QPen(rpmColor, 2));
    painter.setBrush(QBrush(rpmColor));
    painter.drawEllipse(legendX, legendY + 20, 6, 6);
    painter.setPen(QPen(Qt::white, 1));
    painter.drawText(legendX + 10, legendY + 24, 80, 15, Qt::AlignLeft | Qt::AlignVCenter, "Fan Status");
}

QPointF FanCurveWidget::dataToPixel(const QPointF &dataPoint)
{
    QRect graphRect = rect().adjusted(m_marginLeft, m_marginTop, -m_marginRight, -m_marginBottom);
    
    double x = graphRect.left() + (dataPoint.x() - m_tempMin) / (m_tempMax - m_tempMin) * graphRect.width();
    double y = graphRect.bottom() - (dataPoint.y() - m_rpmMin) / (m_rpmMax - m_rpmMin) * graphRect.height();
    
    return QPointF(x, y);
}

QPointF FanCurveWidget::pixelToData(const QPointF &pixelPoint)
{
    QRect graphRect = rect().adjusted(m_marginLeft, m_marginTop, -m_marginRight, -m_marginBottom);
    
    double temp = m_tempMin + (pixelPoint.x() - graphRect.left()) / graphRect.width() * (m_tempMax - m_tempMin);
    double rpm = m_rpmMin + (graphRect.bottom() - pixelPoint.y()) / graphRect.height() * (m_rpmMax - m_rpmMin);
    
    return QPointF(temp, rpm);
}

void FanCurveWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        QPointF clickPoint = event->pos();
        QPointF dataPoint = pixelToData(clickPoint);
        
        // Check if clicking near a data point
        for (int i = 0; i < m_curvePoints.size(); ++i) {
            QPointF point = dataToPixel(m_curvePoints[i]);
            if (QLineF(clickPoint, point).length() < 10) {
                m_dragging = true;
                m_draggedPoint = i;
                break;
            }
        }
    }
}

void FanCurveWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging && m_draggedPoint >= 0) {
        QPointF clickPoint = event->pos();
        QPointF dataPoint = pixelToData(clickPoint);
        
        // Clamp values to valid ranges
        dataPoint.setX(qMax(m_tempMin, qMin(m_tempMax, dataPoint.x())));
        dataPoint.setY(qMax(m_rpmMin, qMin(m_rpmMax, dataPoint.y())));
        
        m_curvePoints[m_draggedPoint] = dataPoint;
        update();
    }
}

void FanCurveWidget::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    m_dragging = false;
    m_draggedPoint = -1;
}

int FanCurveWidget::calculateRPMForTemperature(int temperature)
{
    // Calculate RPM based on the current curve profile
    if (m_curvePoints.size() < 2) {
        return 0;
    }
    
    // Find the two points that bracket the temperature
    for (int i = 0; i < m_curvePoints.size() - 1; ++i) {
        double temp1 = m_curvePoints[i].x();
        double rpm1 = m_curvePoints[i].y();
        double temp2 = m_curvePoints[i + 1].x();
        double rpm2 = m_curvePoints[i + 1].y();
        
        if (temperature >= temp1 && temperature <= temp2) {
            // Linear interpolation between the two points
            double ratio = (temperature - temp1) / (temp2 - temp1);
            double rpm = rpm1 + ratio * (rpm2 - rpm1);
            return static_cast<int>(rpm);
        }
    }
    
    // If temperature is outside the curve range, use the closest point
    if (temperature <= m_curvePoints.first().x()) {
        return static_cast<int>(m_curvePoints.first().y());
    } else {
        return static_cast<int>(m_curvePoints.last().y());
    }
}
