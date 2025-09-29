#ifndef FANWIDGET_H
#define FANWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPropertyAnimation>

class FanWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QColor fanColor READ fanColor WRITE setFanColor)

public:
    explicit FanWidget(int port, int fan, QWidget *parent = nullptr);
    
    void setRPM(int rpm);
    void setProfile(const QString &profile);
    void setLighting(const QString &effect);
    void setColor(const QColor &color);
    void setActive(bool active);
    
    QColor fanColor() const { return m_fanColor; }
    void setFanColor(const QColor &color);

signals:
    void clicked();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    void setupUI();
    void updateAnimation();
    
    int m_port;
    int m_fan;
    int m_rpm;
    QString m_profile;
    QString m_lighting;
    QColor m_fanColor;
    bool m_active;
    
    QVBoxLayout *m_layout;
    QLabel *m_rpmLabel;
    QLabel *m_lightingLabel;
    
    QPropertyAnimation *m_colorAnimation;
};

#endif // FANWIDGET_H
