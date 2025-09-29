#include "customslider.h"

CustomSlider::CustomSlider(const QString &label, QWidget *parent)
    : QWidget(parent)
    , m_labelText(label)
{
    setupUI();
}

void CustomSlider::setupUI()
{
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(10);
    
    // Label
    m_label = new QLabel(m_labelText);
    m_label->setObjectName("sliderLabel");
    m_label->setFixedWidth(80);
    
    // Slider
    m_slider = new QSlider(Qt::Horizontal);
    m_slider->setObjectName("customSlider");
    m_slider->setRange(0, 100);
    m_slider->setValue(75);
    
    // Value label
    m_valueLabel = new QLabel("75%");
    m_valueLabel->setObjectName("sliderValue");
    m_valueLabel->setFixedWidth(40);
    m_valueLabel->setAlignment(Qt::AlignCenter);
    
    m_layout->addWidget(m_label);
    m_layout->addWidget(m_slider);
    m_layout->addWidget(m_valueLabel);
    
    // Connect signals
    connect(m_slider, &QSlider::valueChanged, this, &CustomSlider::onSliderValueChanged);
    
    // Apply styles
    setStyleSheet(R"(
        #sliderLabel {
            color: #cccccc;
            font-size: 12px;
            font-weight: bold;
        }
        
        #customSlider {
            background-color: transparent;
        }
        
        #customSlider::groove:horizontal {
            background-color: #404040;
            height: 6px;
            border-radius: 3px;
        }
        
        #customSlider::handle:horizontal {
            background-color: #2a82da;
            width: 16px;
            height: 16px;
            border-radius: 8px;
            margin: -5px 0;
        }
        
        #customSlider::sub-page:horizontal {
            background-color: #2a82da;
            border-radius: 3px;
        }
        
        #sliderValue {
            color: #ffffff;
            font-size: 12px;
            font-weight: bold;
        }
    )");
}

void CustomSlider::setValue(int value)
{
    m_slider->setValue(value);
}

int CustomSlider::value() const
{
    return m_slider->value();
}

void CustomSlider::setRange(int min, int max)
{
    m_slider->setRange(min, max);
}

void CustomSlider::onSliderValueChanged(int value)
{
    m_valueLabel->setText(QString::number(value) + "%");
    emit valueChanged(value);
}
