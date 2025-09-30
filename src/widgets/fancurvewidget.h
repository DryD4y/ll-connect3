#ifndef FANCURVEWIDGET_H
#define FANCURVEWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QVector>
#include <QPointF>

class FanCurveWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FanCurveWidget(QWidget *parent = nullptr);
    
    void setProfile(const QString &profile);
    void setCurrentTemperature(int temperature);
    void setCurrentRPM(int rpm);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    void setupCurveData();
    void drawGrid(QPainter &painter);
    void drawAxes(QPainter &painter);
    void drawCurve(QPainter &painter);
    void drawDataPoints(QPainter &painter);
    void drawCurrentLine(QPainter &painter);
    QPointF dataToPixel(const QPointF &dataPoint);
    QPointF pixelToData(const QPointF &pixelPoint);
    
    QString m_profile;
    int m_currentTemperature;
    int m_currentRPM;
    
    // Graph dimensions and margins
    int m_marginLeft;
    int m_marginRight;
    int m_marginTop;
    int m_marginBottom;
    
    // Data ranges
    double m_tempMin, m_tempMax;
    double m_rpmMin, m_rpmMax;
    
    // Curve data points (temperature, rpm)
    QVector<QPointF> m_curvePoints;
    
    // Interactive editing
    bool m_dragging;
    int m_draggedPoint;
    
    // Colors
    QColor m_backgroundColor;
    QColor m_gridColor;
    QColor m_axisColor;
    QColor m_curveColor;
    QColor m_pointColor;
    QColor m_currentLineColor;
};

#endif // FANCURVEWIDGET_H
