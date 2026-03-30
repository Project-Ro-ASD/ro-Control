cmake_minimum_required(VERSION 3.22)

project(ro-control
    VERSION 0.2.1
    DESCRIPTION "Smart NVIDIA Driver Manager & System Monitor for Linux"
    HOMEPAGE_URL "https://github.com/Project-Ro-ASD/ro-Control"
    LANGUAGES CXX
)

set(RO_CONTROL_POLICY_ID "io.github.ProjectRoASD.rocontrol.manage-drivers")
set(RO_CONTROL_HELPER_NAME "ro-control-helper")
set(RO_CONTROL_HELPER_BUILD_PATH
    "${CMAKE_CURRENT_BINARY_DIR}/${RO_CONTROL_HELPER_NAME}"
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
    Qml
    Quick          # Qt Quick / QML engine
    QuickControls2 # QML controls (Button, Slider, etc.)
    Widgets        # QApplication base
    DBus           # System D-Bus communication
)

find_package(Qt6 QUIET COMPONENTS LinguistTools)

option(REQUIRE_TRANSLATIONS
    "Fail configuration when Qt Linguist tools are unavailable"
    ON
)

if(REQUIRE_TRANSLATIONS AND NOT Qt6LinguistTools_FOUND)
    message(FATAL_ERROR
        "Qt6 LinguistTools were not found. Install the Qt tools development "
        "package or configure with -DREQUIRE_TRANSLATIONS=OFF for local-only builds."
    )
endif()

qt_standard_project_setup()

if(COMMAND qt_policy)
    qt_policy(SET QTP0001 NEW)
    qt_policy(SET QTP0004 NEW)
endif()

# ─── Source Files ─────────────────────────────────────────────────────────────
set(BACKEND_SOURCES
    src/backend/nvidia/detector.cpp
    src/backend/nvidia/installer.cpp
    src/backend/nvidia/updater.cpp
    src/backend/nvidia/versionparser.cpp
    src/backend/monitor/gpumonitor.cpp
    src/backend/monitor/cpumonitor.cpp
    src/backend/monitor/rammonitor.cpp
    src/backend/system/polkit.cpp
    src/backend/system/dnfmanager.cpp
    src/backend/system/commandrunner.cpp
    src/backend/system/sessionutil.cpp
    src/backend/system/capabilityprobe.cpp
    src/backend/system/languagemanager.cpp
    src/backend/system/uipreferencesmanager.cpp
    src/backend/system/systeminfoprovider.cpp
)

set(APP_SOURCES
    src/main.cpp
    src/cli/cli.cpp
)

# ─── Install Paths ────────────────────────────────────────────────────────────
include(GNUInstallDirs)

set(RO_CONTROL_HELPER_INSTALL_PATH
    "${CMAKE_INSTALL_FULL_LIBEXECDIR}/${RO_CONTROL_HELPER_NAME}"
)
set(RO_CONTROL_POLICY_BUILD_PATH
    "${CMAKE_CURRENT_BINARY_DIR}/io.github.ProjectRoASD.rocontrol.policy"
)

# ─── QML Resources ────────────────────────────────────────────────────────────
add_library(ro-control-backend STATIC ${BACKEND_SOURCES})

target_link_libraries(ro-control-backend PUBLIC
    Qt6::Core
    Qt6::DBus
    Qt6::Qml
)

target_compile_options(ro-control-backend PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O2>
)

target_include_directories(ro-control-backend PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/backend
)

target_compile_definitions(ro-control-backend PUBLIC
    RO_CONTROL_POLICY_ID="${RO_CONTROL_POLICY_ID}"
    RO_CONTROL_HELPER_BUILD_PATH="${RO_CONTROL_HELPER_BUILD_PATH}"
    RO_CONTROL_HELPER_INSTALL_PATH="${RO_CONTROL_HELPER_INSTALL_PATH}"
)

qt_add_executable(ro-control ${APP_SOURCES})

set_source_files_properties(src/qml/assets/ro-control-logo.svg PROPERTIES
    QT_RESOURCE_ALIAS "assets/ro-control-logo.svg"
)

set_source_files_properties(src/qml/assets/ro-control-logo.png PROPERTIES
    QT_RESOURCE_ALIAS "assets/ro-control-logo.png"
)

qt_add_qml_module(ro-control
    URI "rocontrol"
    VERSION 1.0
    QML_FILES
        src/qml/Main.qml
        src/qml/pages/DriverPage.qml
        src/qml/pages/MonitorPage.qml
        src/qml/pages/SettingsPage.qml
        src/qml/components/InfoBadge.qml
        src/qml/components/ActionButton.qml
        src/qml/components/DetailRow.qml
        src/qml/components/SectionPanel.qml
        src/qml/components/StatusBanner.qml
        src/qml/components/StatCard.qml
        src/qml/components/SidebarMenu.qml
    RESOURCES
        src/qml/assets/ro-control-logo.svg
        src/qml/assets/ro-control-logo.png
)

if(Qt6LinguistTools_FOUND)
    # English strings ship directly from source code. Keep the English TS file
    # for catalog maintenance, but only compile non-source locales into .qm.
    set(TS_FILES
        i18n/ro-control_de.ts
        i18n/ro-control_es.ts
        i18n/ro-control_tr.ts
    )

    qt_add_translations(ro-control
        TS_FILES ${TS_FILES}
        RESOURCE_PREFIX "/i18n"
    )
endif()

# ─── Link Qt Libraries ────────────────────────────────────────────────────────
target_link_libraries(ro-control PRIVATE
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Widgets
    ro-control-backend
)

# ─── Compiler Warnings ────────────────────────────────────────────────────────
target_compile_options(ro-control PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O2>
)

# ─── Include Directories ──────────────────────────────────────────────────────
target_include_directories(ro-control PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/backend
)

target_compile_definitions(ro-control PRIVATE
    RO_CONTROL_POLICY_ID="${RO_CONTROL_POLICY_ID}"
    RO_CONTROL_APP_VERSION="${PROJECT_VERSION}"
)

configure_file(
    data/helpers/ro-control-helper.sh.in
    ${RO_CONTROL_HELPER_BUILD_PATH}
    @ONLY
    NEWLINE_STYLE UNIX
)
execute_process(COMMAND chmod +x ${RO_CONTROL_HELPER_BUILD_PATH})

configure_file(
    data/polkit/io.github.ProjectRoASD.rocontrol.policy.in
    ${RO_CONTROL_POLICY_BUILD_PATH}
    @ONLY
    NEWLINE_STYLE UNIX
)

install(TARGETS ro-control
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(PROGRAMS ${RO_CONTROL_HELPER_BUILD_PATH}
    DESTINATION ${CMAKE_INSTALL_LIBEXECDIR}
    RENAME ${RO_CONTROL_HELPER_NAME}
)

install(FILES data/icons/io.github.projectroasd.rocontrol.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
)

install(FILES
    data/icons/hicolor/scalable/apps/ro-control.svg
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps
)

install(FILES
    data/icons/hicolor/256x256/apps/ro-control.png
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/256x256/apps
)

install(FILES data/icons/io.github.projectroasd.rocontrol.metainfo.xml
    DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo
)

install(FILES ${RO_CONTROL_POLICY_BUILD_PATH}
    DESTINATION ${CMAKE_INSTALL_DATADIR}/polkit-1/actions
)

install(FILES docs/man/ro-control.1
    DESTINATION ${CMAKE_INSTALL_DATADIR}/man/man1
)

install(FILES data/completions/ro-control.bash
    DESTINATION ${CMAKE_INSTALL_DATADIR}/bash-completion/completions
    RENAME ro-control
)

install(FILES data/completions/_ro-control
    DESTINATION ${CMAKE_INSTALL_DATADIR}/zsh/site-functions
)

install(FILES data/completions/ro-control.fish
    DESTINATION ${CMAKE_INSTALL_DATADIR}/fish/vendor_completions.d
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
message(STATUS "  CPU arch   : ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "  Qt version : ${Qt6_VERSION}")
message(STATUS "  Install to : ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")
