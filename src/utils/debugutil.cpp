/*---------------------------------------------------------*\
||| debugutil.cpp                                           |
|||                                                         |
|||   Debug utility implementation                         |
|||                                                         |
|||   This file is part of the LL-Connect 3 project        |
|||   SPDX-License-Identifier: GPL-2.0-or-later            |
\*---------------------------------------------------------*/

#include "debugutil.h"
#include <QSettings>

namespace DebugUtil {

bool isDebugEnabled() {
    QSettings settings("LianLi", "LConnect3");
    return settings.value("Debug/Enabled", false).toBool();
}

bool isDebugCategoryEnabled(const char* category) {
    QSettings settings("LianLi", "LConnect3");
    // First check if debug mode is enabled
    if (!settings.value("Debug/Enabled", false).toBool()) {
        return false;
    }
    // Then check if the specific category is enabled
    QString categoryKey = QString("Debug/%1").arg(category);
    return settings.value(categoryKey, false).toBool();
}

} // namespace DebugUtil

