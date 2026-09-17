cmake_minimum_required(VERSION 3.22)

project(ro-control
    VERSION 1.3.1
    DESCRIPTION "Linux hardware monitor, cooling controller, and NVIDIA driver manager"
    HOMEPAGE_URL "https://github.com/Project-Ro-ASD/ro-Control"
    LANGUAGES CXX
)

set(CPACK_PACKAGE_FILE_NAME
    "${PROJECT_NAME}-${PROJECT_VERSION}-${CMAKE_SYSTEM_PROCESSOR}"
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

# ─── LTO (Release) ────────────────────────────────────────────────────────────
option(ENABLE_LTO "Enable link-time optimization for release builds" ON)
if(ENABLE_LTO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT has_lto)
    if(has_lto AND CMAKE_BUILD_TYPE STREQUAL "Release")
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
endif()

# ─── Sanitizers (Debug) ──────────────────────────────────────────────────────
option(ENABLE_ASAN "Enable AddressSanitizer for debug builds" OFF)
option(ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer for debug builds" OFF)

if(ENABLE_ASAN OR ENABLE_UBSAN)
    set(SANITIZER_FLAGS "")
    if(ENABLE_ASAN)
        string(APPEND SANITIZER_FLAGS " -fsanitize=address")
    endif()
    if(ENABLE_UBSAN)
        string(APPEND SANITIZER_FLAGS " -fsanitize=undefined")
    endif()
    string(STRIP "${SANITIZER_FLAGS}" SANITIZER_FLAGS)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${SANITIZER_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} ${SANITIZER_FLAGS}")
endif()

# ─── Qt6 Setup ───────────────────────────────────────────────────────────────
# Qt's MOC, UIC, RCC run automatically
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Concurrent
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
    src/backend/fan/fancontroller.cpp
    src/backend/power/powercontroller.cpp
    src/backend/system/healthguard.cpp
    src/backend/system/dbusservice.cpp
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
    Qt6::Concurrent
    Qt6::DBus
    Qt6::Gui
    Qt6::Qml
)

target_compile_options(ro-control-backend PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O3 -fvisibility=hidden -fvisibility-inlines-hidden -fdata-sections -ffunction-sections>
)

if(NOT APPLE)
    target_link_options(ro-control-backend PRIVATE
        $<$<CONFIG:Release>:-Wl,--gc-sections>
    )
endif()

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

set_source_files_properties(src/qml/assets/icon-language.svg PROPERTIES
    QT_RESOURCE_ALIAS "assets/icon-language.svg"
)

set_source_files_properties(src/qml/assets/icon-refresh.svg PROPERTIES
    QT_RESOURCE_ALIAS "assets/icon-refresh.svg"
)

set_source_files_properties(src/qml/assets/icon-refresh-light.svg PROPERTIES
    QT_RESOURCE_ALIAS "assets/icon-refresh-light.svg"
)

set_source_files_properties(src/qml/assets/icon-theme.svg PROPERTIES
    QT_RESOURCE_ALIAS "assets/icon-theme.svg"
)

set_source_files_properties(src/qml/assets/icon-settings.svg PROPERTIES
    QT_RESOURCE_ALIAS "assets/icon-settings.svg"
)

qt_add_qml_module(ro-control
    URI "rocontrol"
    VERSION 1.0
    QML_FILES
        src/qml/Main.qml
        src/qml/pages/DriverPage.qml
        src/qml/pages/MonitorPage.qml
        src/qml/pages/FanPage.qml
        src/qml/pages/SystemPage.qml
        src/qml/components/InfoBadge.qml
        src/qml/components/RefreshToolButton.qml
        src/qml/components/ActionButton.qml
        src/qml/components/DetailRow.qml
        src/qml/components/SectionPanel.qml
        src/qml/components/StatusBanner.qml
        src/qml/components/StatCard.qml
        src/qml/components/SidebarMenu.qml
        src/qml/components/FanSettingsPopup.qml
        src/qml/components/FanRescanPopup.qml
    RESOURCES
        src/qml/assets/ro-control-logo.svg
        src/qml/assets/icon-language.svg
        src/qml/assets/icon-refresh.svg
        src/qml/assets/icon-refresh-light.svg
        src/qml/assets/icon-theme.svg
        src/qml/assets/icon-settings.svg
)

# Keep QML cache generation on the public bytecode path so RPM dependencies do
# not pick up Qt private ABI symbols from AOT-generated C++.
set_target_properties(ro-control PROPERTIES
    QT_QMLCACHEGEN_ARGUMENTS "--only-bytecode"
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
    $<$<CONFIG:Release>:-O3 -fvisibility=hidden -fvisibility-inlines-hidden -fdata-sections -ffunction-sections>
)

if(APPLE)
    target_link_options(ro-control PRIVATE
        $<$<CONFIG:Release>:-Wl,-dead_strip>
    )
else()
    target_link_options(ro-control PRIVATE
        $<$<CONFIG:Release>:-Wl,--gc-sections -Wl,-O1 -Wl,--as-needed>
    )
endif()

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

file(CHMOD ${RO_CONTROL_HELPER_BUILD_PATH}
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                GROUP_READ GROUP_EXECUTE
                WORLD_READ WORLD_EXECUTE
)

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
    data/icons/hicolor/scalable/apps/ro-control.svg
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps
    RENAME io.github.projectroasd.rocontrol.svg
)

install(FILES
    data/icons/hicolor/256x256/apps/ro-control.png
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/256x256/apps
)

install(FILES
    data/icons/hicolor/256x256/apps/ro-control.png
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/256x256/apps
    RENAME io.github.projectroasd.rocontrol.png
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

# ─── Systemd Service Units ───────────────────────────────────────────────────
configure_file(
    data/systemd/ro-control.service.in
    ${CMAKE_CURRENT_BINARY_DIR}/ro-control.service
    @ONLY
)

configure_file(
    data/systemd/ro-control-user.service.in
    ${CMAKE_CURRENT_BINARY_DIR}/ro-control-user.service
    @ONLY
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ro-control.service
    DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/systemd/system
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ro-control-user.service
    DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/systemd/user
    RENAME ro-control.service
)

# ─── Formatting ──────────────────────────────────────────────────────────────
find_program(CLANG_FORMAT_EXE clang-format)
if(CLANG_FORMAT_EXE)
    file(GLOB_RECURSE ALL_SOURCE_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.h
    )
    add_custom_target(format
        COMMAND ${CLANG_FORMAT_EXE} -i ${ALL_SOURCE_FILES}
        COMMENT "Running clang-format on all C++ source files"
    )
    add_custom_target(check-format
        COMMAND ${CLANG_FORMAT_EXE} --dry-run --Werror ${ALL_SOURCE_FILES}
        COMMENT "Verifying clang-format on all C++ source files"
    )
endif()

# ─── Tests ───────────────────────────────────────────────────────────────────
option(BUILD_TESTS "Build unit tests" OFF)

# Also honour the CMake standard BUILD_TESTING convention
if(BUILD_TESTING AND NOT BUILD_TESTS)
    set(BUILD_TESTS ON CACHE BOOL "Build unit tests" FORCE)
endif()

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

include(CPack)
