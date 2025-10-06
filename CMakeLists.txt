cmake_minimum_required(VERSION 3.16)
project(LConnect3 VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6 components
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# Find libusb
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBUSB REQUIRED libusb-1.0)

# Find hidapi
find_package(PkgConfig REQUIRED)
pkg_check_modules(HIDAPI REQUIRED hidapi-hidraw)

# Create resources file
qt6_add_resources(RESOURCES resources.qrc)

# Add USB controller subdirectory
add_subdirectory(src/usb)

# Add Qt integration
add_library(lian_li_qt_integration
    src/lian_li_qt_integration.cpp
    src/lian_li_qt_integration.h
)

# Enable MOC for Qt integration
set_target_properties(lian_li_qt_integration PROPERTIES
    AUTOMOC ON
)

target_link_libraries(lian_li_qt_integration
    Qt6::Core
    Qt6::Widgets
    sl_infinity_hid
)

target_include_directories(lian_li_qt_integration
    PUBLIC
        src
    PRIVATE
        src/usb
)

# Enable Qt6 automatic MOC, UIC, and RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Source files
set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.ui
    src/pages/systeminfopage.cpp
    src/pages/fanprofilepage.cpp
    src/pages/lightingpage.cpp
    src/pages/slinfinitypage.cpp
    src/pages/settingspage.cpp
    src/widgets/fanwidget.cpp
    src/widgets/fancurvewidget.cpp
    src/widgets/monitoringcard.cpp
    src/widgets/customslider.cpp
    src/widgets/fanlightingwidget.cpp
)

# Header files
set(HEADERS
    src/mainwindow.h
    src/pages/systeminfopage.h
    src/pages/fanprofilepage.h
    src/pages/lightingpage.h
    src/pages/slinfinitypage.h
    src/pages/settingspage.h
    src/widgets/fanwidget.h
    src/widgets/fancurvewidget.h
    src/widgets/monitoringcard.h
    src/widgets/customslider.h
    src/widgets/fanlightingwidget.h
)

# Create executable
add_executable(LConnect3 ${SOURCES} ${HEADERS} ${RESOURCES})

# Include directories
target_include_directories(LConnect3 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Link Qt6 libraries
target_link_libraries(LConnect3
    Qt6::Core
    Qt6::Widgets
    lian_li_qt_integration
    lian_li_sl_infinity_controller
    ${HIDAPI_LIBRARIES}
)

# Add hidapi include directories
target_include_directories(LConnect3 PRIVATE ${HIDAPI_INCLUDE_DIRS})

# Set target properties
set_target_properties(LConnect3 PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)


# Enable High DPI support
if(WIN32)
    set_target_properties(LConnect3 PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
    # Add High DPI manifest for Windows
    target_compile_definitions(LConnect3 PRIVATE
        QT_AUTO_SCREEN_SCALE_FACTOR=1
        QT_ENABLE_HIGHDPI_SCALING=1
    )
endif()

# Set High DPI properties for all platforms
target_compile_definitions(LConnect3 PRIVATE
    QT_AUTO_SCREEN_SCALE_FACTOR=1
    QT_ENABLE_HIGHDPI_SCALING=1
)

# Install target
install(TARGETS LConnect3
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)
