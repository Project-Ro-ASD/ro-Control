cmake_minimum_required(VERSION 3.22)

project(ro-control
    VERSION 0.1.0
    DESCRIPTION "Smart NVIDIA Driver Manager & System Monitor for Linux"
    HOMEPAGE_URL "https://github.com/Acik-Kaynak-Gelistirme-Toplulugu/ro-Control"
    LANGUAGES CXX
)

# ─── C++ Standard ────────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ─── Build type default ───────────────────────────────────────────────────────
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

# ─── Qt6 Setup ───────────────────────────────────────────────────────────────
# Qt's MOC, UIC, RCC run automatically
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Quick          # Qt Quick / QML engine
    QuickControls2 # QML controls (Button, Slider, etc.)
    Widgets        # QApplication base
    DBus           # System D-Bus communication
)

qt_standard_project_setup()

# ─── Source Files ─────────────────────────────────────────────────────────────
set(BACKEND_SOURCES
    src/backend/nvidia/detector.cpp
    src/backend/nvidia/installer.cpp
    src/backend/nvidia/updater.cpp
    src/backend/monitor/gpumonitor.cpp
    src/backend/monitor/cpumonitor.cpp
    src/backend/monitor/rammonitor.cpp
    src/backend/system/polkit.cpp
    src/backend/system/dnfmanager.cpp
    src/backend/system/commandrunner.cpp
)

set(APP_SOURCES
    src/main.cpp
    ${BACKEND_SOURCES}
)

# ─── QML Resources ────────────────────────────────────────────────────────────
qt_add_executable(ro-control ${APP_SOURCES})

qt_add_qml_module(ro-control
    URI "rocontrol"
    VERSION 1.0
    QML_FILES
        src/qml/Main.qml
        src/qml/pages/DriverPage.qml
        src/qml/pages/MonitorPage.qml
        src/qml/pages/SettingsPage.qml
        src/qml/components/StatCard.qml
        src/qml/components/SidebarMenu.qml
)

# ─── Include Directories ──────────────────────────────────────────────────────
target_include_directories(ro-control PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/backend
)

# ─── Link Qt Libraries ────────────────────────────────────────────────────────
target_link_libraries(ro-control PRIVATE
    Qt6::Core
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Widgets
    Qt6::DBus
)

# ─── Compiler Warnings ────────────────────────────────────────────────────────
target_compile_options(ro-control PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O2>
)

# ─── Install Targets ──────────────────────────────────────────────────────────
include(GNUInstallDirs)

install(TARGETS ro-control
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES data/icons/ro-control.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
)

install(DIRECTORY data/icons/
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons
)

install(FILES data/icons/ro-control.metainfo.xml
    DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo
)

install(FILES data/polkit/com.github.AcikKaynakGelistirmeToplulugu.rocontrol.policy
    DESTINATION ${CMAKE_INSTALL_DATADIR}/polkit-1/actions
)

# ─── Tests ───────────────────────────────────────────────────────────────────
option(BUILD_TESTS "Build unit tests" OFF)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# ─── Summary ─────────────────────────────────────────────────────────────────
message(STATUS "")
message(STATUS "ro-control ${PROJECT_VERSION}")
message(STATUS "  Build type : ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ std    : C++${CMAKE_CXX_STANDARD}")
message(STATUS "  Qt version : ${Qt6_VERSION}")
message(STATUS "  Install to : ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")
