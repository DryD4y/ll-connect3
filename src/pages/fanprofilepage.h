#ifndef FANPROFILEPAGE_H
#define FANPROFILEPAGE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QSlider>
#include <QSpinBox>
#include <QTableWidget>
#include <QGroupBox>
#include <QRadioButton>
#include <QCheckBox>
#include <QWidget>

class FanProfilePage : public QWidget
{
    Q_OBJECT

public:
    explicit FanProfilePage(QWidget *parent = nullptr);

private slots:
    void onProfileChanged();
    void onApplyToAll();
    void onDefault();
    void onStartStopToggled();
    void updateFanData();

private:
    void setupUI();
    void setupFanTable();
    void setupFanCurve();
    void setupControls();
    void updateFanCurve();
    
    QVBoxLayout *m_mainLayout;
    QVBoxLayout *m_contentLayout;
    QVBoxLayout *m_leftLayout;
    QVBoxLayout *m_rightLayout;
    
    // Fan table
    QTableWidget *m_fanTable;
    
    // Fan curve (simplified)
    QWidget *m_fanCurveWidget;
    
    // Controls
    QGroupBox *m_profileGroup;
    QRadioButton *m_quietRadio;
    QRadioButton *m_stdSpRadio;
    QRadioButton *m_highSpRadio;
    QRadioButton *m_fullSpRadio;
    QRadioButton *m_customRadio;
    
    QCheckBox *m_startStopCheck;
    
    QLabel *m_tempLabel;
    QSpinBox *m_tempSpinBox;
    QLabel *m_rpmLabel;
    QSpinBox *m_rpmSpinBox;
    
    QPushButton *m_applyToAllBtn;
    QPushButton *m_defaultBtn;
    
    // Update timer
    QTimer *m_updateTimer;
};

#endif // FANPROFILEPAGE_H
