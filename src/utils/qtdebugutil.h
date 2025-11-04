/*---------------------------------------------------------*\
||| qtdebugutil.h                                           |
|||                                                         |
|||   Qt-specific debug utility for conditional logging    |
|||                                                         |
|||   This file is part of the LL-Connect 3 project        |
|||   SPDX-License-Identifier: GPL-2.0-or-later            |
\*---------------------------------------------------------*/

#pragma once

#include "debugutil.h"
#include <QDebug>

namespace DebugUtil {

// Debug qDebug (only prints if debug mode is enabled)
// Helper to expand arguments
inline void debugLog_impl(QDebug dbg) {
    // Base case: do nothing
}

template<typename T, typename... Args>
inline void debugLog_impl(QDebug dbg, T&& value, Args&&... args) {
    dbg << std::forward<T>(value);
    debugLog_impl(dbg, std::forward<Args>(args)...);
}

template<typename... Args>
inline void debugLog(Args&&... args) {
    if (isDebugEnabled()) {
        QDebug dbg = qDebug();
        debugLog_impl(dbg, std::forward<Args>(args)...);
    }
}

// Category-specific debug logging
template<typename... Args>
inline void debugLogCategory(const char* category, Args&&... args) {
    if (isDebugCategoryEnabled(category)) {
        QDebug dbg = qDebug();
        debugLog_impl(dbg, std::forward<Args>(args)...);
    }
}

} // namespace DebugUtil

// Convenience macro for Qt code
#define DEBUG_LOG(...) DebugUtil::debugLog(__VA_ARGS__)

// Convenience macro for category-specific debug logging
#define DEBUG_LOG_CATEGORY(category, ...) DebugUtil::debugLogCategory(category, __VA_ARGS__)

