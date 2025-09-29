#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QTabWidget>
#include <QScrollArea>
#include <QCheckBox>

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
QT_END_NAMESPACE

class SystemInfoPage;
class FanProfilePage;
class LightingPage;
class SLInfinityPage;
class SettingsPage;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onNavigationClicked();
    void onTabChanged(int index);

private:
    void setupUI();
    void setupSidebar();
    void setupTopTabs();
    void setupMainContent();
    void applyStyles();
    
    // UI Components
    QWidget *m_centralWidget;
    QHBoxLayout *m_mainLayout;
    
    // Sidebar
    QFrame *m_sidebar;
    QVBoxLayout *m_sidebarLayout;
    QFrame *m_logoBadge;
    QLabel *m_logoInitialLabel;
    QLabel *m_logoLabel;
    QPushButton *m_systemInfoBtn;
    QPushButton *m_fanProfileBtn;
    QPushButton *m_lightingBtn;
    QPushButton *m_slInfinityBtn;
    QPushButton *m_settingsBtn;
    QLabel *m_versionLabel;
    
    // Top area
    QWidget *m_topWidget;
    QHBoxLayout *m_topLayout;
    QTabWidget *m_tabWidget;
    QLabel *m_toggleLabel;
    QCheckBox *m_floatingToggle;
    QPushButton *m_exportBtn;
    QPushButton *m_importBtn;
    
    // Main content
    QStackedWidget *m_contentStack;
    
    // Pages
    SystemInfoPage *m_systemInfoPage;
    FanProfilePage *m_fanProfilePage;
    LightingPage *m_lightingPage;
    SLInfinityPage *m_slInfinityPage;
    SettingsPage *m_settingsPage;
    
    // Current page tracking
    int m_currentPage;
};

#endif // MAINWINDOW_H
