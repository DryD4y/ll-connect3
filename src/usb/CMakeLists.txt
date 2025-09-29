cmake_minimum_required(VERSION 3.16)

# USB Controller Library
add_library(lian_li_usb_controller
    lian_li_usb_controller.cpp
    lian_li_usb_controller.h
)

# SL Infinity HID Controller Library
if(HIDAPI_FOUND)
    add_library(lian_li_sl_infinity_controller
        lian_li_sl_infinity_controller.cpp
        lian_li_sl_infinity_controller.h
    )
    
    # Simple HID controller (no external dependencies)
    add_library(sl_infinity_hid
        sl_infinity_hid.cpp
        sl_infinity_hid.h
    )
endif()

# Find libusb and hidapi
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBUSB REQUIRED libusb-1.0)

# Find hidapi using pkg-config
pkg_check_modules(HIDAPI hidapi-hidraw)
if(HIDAPI_FOUND)
    message(STATUS "Found hidapi via pkg-config: ${HIDAPI_LIBRARIES}")
else()
    # Try alternative method
    find_library(HIDAPI_LIBRARIES hidapi-hidraw)
    if(HIDAPI_LIBRARIES)
        set(HIDAPI_FOUND TRUE)
        message(STATUS "Found hidapi via find_library: ${HIDAPI_LIBRARIES}")
    else()
        message(STATUS "hidapi not found - SL Infinity controller will not be built")
    endif()
endif()

# Link libusb to USB controller
target_link_libraries(lian_li_usb_controller
    ${LIBUSB_LIBRARIES}
)

# Link hidapi to SL Infinity controller (disabled)
# if(HIDAPI_FOUND)
#     target_link_libraries(lian_li_sl_infinity_controller
#         ${HIDAPI_LIBRARIES}
#     )
# endif()

# Include directories
target_include_directories(lian_li_usb_controller
    PUBLIC
        ${LIBUSB_INCLUDE_DIRS}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# if(HIDAPI_FOUND)
#     target_include_directories(lian_li_sl_infinity_controller
#         PUBLIC
#             ${HIDAPI_INCLUDE_DIRS}
#         PRIVATE
#             ${CMAKE_CURRENT_SOURCE_DIR}
#     )
# endif()

# Compiler flags
target_compile_options(lian_li_usb_controller
    PRIVATE
        ${LIBUSB_CFLAGS_OTHER}
)

# if(HIDAPI_FOUND)
#     target_compile_options(lian_li_sl_infinity_controller
#         PRIVATE
#             ${HIDAPI_CFLAGS_OTHER}
#     )
# endif()

# Include directories for sl_infinity_hid (always available)
target_include_directories(sl_infinity_hid
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Include directories for lian_li_sl_infinity_controller (only if built)
if(HIDAPI_FOUND)
    target_include_directories(lian_li_sl_infinity_controller
        PRIVATE
            /usr/include/hidapi
            ${CMAKE_CURRENT_SOURCE_DIR}
    )
endif()
