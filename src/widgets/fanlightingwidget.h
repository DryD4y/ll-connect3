#ifndef FANLIGHTINGWIDGET_H
#define FANLIGHTINGWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QPainter>
#include <QColor>
#include <QString>

class FanLightingWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FanLightingWidget(QWidget *parent = nullptr);
    
    void setEffect(const QString &effect);
    void setSpeed(int speedPercent);
    void setBrightness(int brightnessPercent);
    void setDirection(bool leftToRight);
    void setColor(const QColor &color);
    void setPortColors(const QColor colors[4]);

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void updateAnimation();

private:
    void drawFan(QPainter &painter, const QRect &rect, int fanIndex);
    void drawRainbowEffect(QPainter &painter, const QRect &rect, int fanIndex);
    void drawRainbowMorphEffect(QPainter &painter, const QRect &rect, int fanIndex);
    void drawStaticColorEffect(QPainter &painter, const QRect &rect, int fanIndex);
    void drawBreathingEffect(QPainter &painter, const QRect &rect, int fanIndex);
    void drawMeteorEffect(QPainter &painter, const QRect &rect, int fanIndex);
    void drawRunwayEffect(QPainter &painter, const QRect &rect, int fanIndex);
    
    QColor getRainbowColor(int position, int totalPositions) const;
    QColor applyBrightness(const QColor &color, int brightnessPercent) const;
    
    QString m_effect;
    int m_speed;
    int m_brightness;
    bool m_directionLeft;
    QColor m_color;
    QColor m_portColors[4]; // Colors for each port
    
    QTimer *m_animationTimer;
    int m_animationFrame;
    double m_timeOffset;
};

#endif // FANLIGHTINGWIDGET_H
