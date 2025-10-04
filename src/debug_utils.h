#ifndef DEBUG_UTILS_H
#define DEBUG_UTILS_H

#include <QSettings>
#include <QDebug>

class DebugUtils
{
public:
    static bool isDebugEnabled() {
        static bool initialized = false;
        static bool debugEnabled = false;
        
        if (!initialized) {
            QSettings settings("LianLi", "LConnect3");
            debugEnabled = settings.value("Debug/Enabled", false).toBool();
            initialized = true;
        }
        
        return debugEnabled;
    }
    
    static void setDebugEnabled(bool enabled) {
        QSettings settings("LianLi", "LConnect3");
        settings.setValue("Debug/Enabled", enabled);
        settings.sync();
    }
    
    static void reloadDebugSettings() {
        QSettings settings("LianLi", "LConnect3");
        // Force reload by clearing cache (we'll use a simple approach)
    }
};

// Macro for conditional debug output
#define DEBUG_LOG(msg) \
    if (DebugUtils::isDebugEnabled()) { \
        qDebug() << msg; \
    }

#endif // DEBUG_UTILS_H
