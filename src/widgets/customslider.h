#ifndef CUSTOMSLIDER_H
#define CUSTOMSLIDER_H

#include <QWidget>
#include <QSlider>
#include <QLabel>
#include <QHBoxLayout>

class CustomSlider : public QWidget
{
    Q_OBJECT

public:
    explicit CustomSlider(const QString &label, QWidget *parent = nullptr);
    
    void setValue(int value);
    int value() const;
    void setRange(int min, int max);
    void setTickInterval(int interval);
    void setPageStep(int step);
    void setSnapToIncrements(bool enabled, int increment = 25);

signals:
    void valueChanged(int value);

private slots:
    void onSliderValueChanged(int value);

private:
    void setupUI();
    
    QHBoxLayout *m_layout;
    QLabel *m_label;
    QSlider *m_slider;
    QLabel *m_valueLabel;
    
    QString m_labelText;
    bool m_snapToIncrements;
    int m_increment;
};

#endif // CUSTOMSLIDER_H
