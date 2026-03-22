find_package(Qt6 REQUIRED COMPONENTS Test Qml Quick QuickControls2)

function(ro_control_configure_test target_name)
    target_include_directories(${target_name} PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/backend
    )
endfunction()

# ─── Detector Tests ──────────────────────────────────────────────────────────
qt_add_executable(test_detector
    test_detector.cpp
)

ro_control_configure_test(test_detector)

target_link_libraries(test_detector PRIVATE
    Qt6::Test
    ro-control-backend
)

add_test(NAME test_detector COMMAND test_detector)

# ─── Updater/Version Parser Tests ────────────────────────────────────────────
qt_add_executable(test_updater
    test_updater.cpp
)

ro_control_configure_test(test_updater)

target_link_libraries(test_updater PRIVATE
    Qt6::Test
    ro-control-backend
)

add_test(NAME test_updater COMMAND test_updater)

# ─── Monitor Tests ───────────────────────────────────────────────────────────
qt_add_executable(test_monitor
    test_monitor.cpp
)

ro_control_configure_test(test_monitor)

target_link_libraries(test_monitor PRIVATE
    Qt6::Test
    ro-control-backend
)

add_test(NAME test_monitor COMMAND test_monitor)

# ─── Preferences / Localization Tests ───────────────────────────────────────
qt_add_executable(test_preferences
    test_preferences.cpp
)

ro_control_configure_test(test_preferences)

target_link_libraries(test_preferences PRIVATE
    Qt6::Qml
    Qt6::Test
    ro-control-backend
)

add_test(NAME test_preferences COMMAND test_preferences)

# ─── System Integration Tests ───────────────────────────────────────────────
qt_add_executable(test_system_integration
    test_system_integration.cpp
)

ro_control_configure_test(test_system_integration)

target_link_libraries(test_system_integration PRIVATE
    Qt6::Test
    ro-control-backend
)

add_test(NAME test_system_integration COMMAND test_system_integration)

# ─── Metadata Tests ──────────────────────────────────────────────────────────
qt_add_executable(test_metadata
    test_metadata.cpp
)

target_compile_definitions(test_metadata PRIVATE
    "RO_CONTROL_SOURCE_DIR=\"${CMAKE_SOURCE_DIR}\""
)

target_link_libraries(test_metadata PRIVATE
    Qt6::Core
    Qt6::Test
)

add_test(NAME test_metadata COMMAND test_metadata)

# ─── CLI Tests ───────────────────────────────────────────────────────────────
qt_add_executable(test_cli
    test_cli.cpp
    ${CMAKE_SOURCE_DIR}/src/cli/cli.cpp
)

ro_control_configure_test(test_cli)

target_link_libraries(test_cli PRIVATE
    Qt6::Test
    ro-control-backend
)

add_test(NAME test_cli COMMAND test_cli)

# ─── Driver Page QML Integration Tests ──────────────────────────────────────
qt_add_executable(test_driver_page
    test_driver_page.cpp
)

ro_control_configure_test(test_driver_page)

target_compile_definitions(test_driver_page PRIVATE
    "RO_CONTROL_SOURCE_DIR=\"${CMAKE_SOURCE_DIR}\""
)

target_link_libraries(test_driver_page PRIVATE
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Test
)

add_test(NAME test_driver_page COMMAND test_driver_page)
set_tests_properties(test_driver_page PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_CONTROLS_STYLE=Basic"
)
