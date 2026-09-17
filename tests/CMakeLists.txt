find_package(Qt6 REQUIRED COMPONENTS Test Qml Quick QuickControls2)

set(RO_CONTROL_TEST_ENV
    "QT_QPA_PLATFORM=offscreen;QT_QUICK_CONTROLS_STYLE=Basic"
)

function(ro_control_configure_test target_name)
    target_include_directories(${target_name} PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/backend
    )
endfunction()

function(ro_control_register_test test_name)
    add_test(NAME ${test_name} COMMAND ${test_name})
    set_tests_properties(${test_name} PROPERTIES
        ENVIRONMENT "${RO_CONTROL_TEST_ENV}"
        TIMEOUT 30
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

ro_control_register_test(test_detector)

# ─── Updater/Version Parser Tests ────────────────────────────────────────────
qt_add_executable(test_updater
    test_updater.cpp
)

ro_control_configure_test(test_updater)

target_link_libraries(test_updater PRIVATE
    Qt6::Test
    ro-control-backend
)

ro_control_register_test(test_updater)

# ─── Monitor Tests ───────────────────────────────────────────────────────────
qt_add_executable(test_monitor
    test_monitor.cpp
)

ro_control_configure_test(test_monitor)

target_link_libraries(test_monitor PRIVATE
    Qt6::Test
    ro-control-backend
)

ro_control_register_test(test_monitor)

# ─── Fan Controller Tests ───────────────────────────────────────────────────
qt_add_executable(test_fan_controller
    test_fan_controller.cpp
)

ro_control_configure_test(test_fan_controller)

target_link_libraries(test_fan_controller PRIVATE
    Qt6::Test
    ro-control-backend
)

ro_control_register_test(test_fan_controller)

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

ro_control_register_test(test_preferences)

# ─── System Integration Tests ───────────────────────────────────────────────
qt_add_executable(test_system_integration
    test_system_integration.cpp
)

ro_control_configure_test(test_system_integration)

target_link_libraries(test_system_integration PRIVATE
    Qt6::Test
    ro-control-backend
)

ro_control_register_test(test_system_integration)

# ─── Metadata Tests ──────────────────────────────────────────────────────────
qt_add_executable(test_metadata
    test_metadata.cpp
)

target_compile_definitions(test_metadata PRIVATE
    "RO_CONTROL_SOURCE_DIR=\"${CMAKE_SOURCE_DIR}\""
    "RO_CONTROL_PROJECT_VERSION=\"${PROJECT_VERSION}\""
)

target_link_libraries(test_metadata PRIVATE
    Qt6::Core
    Qt6::Test
)

ro_control_register_test(test_metadata)

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

ro_control_register_test(test_cli)

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

ro_control_register_test(test_driver_page)

# ─── Power Controller Tests ──────────────────────────────────────────────────
qt_add_executable(test_power_controller
    test_power_controller.cpp
)

ro_control_configure_test(test_power_controller)

target_link_libraries(test_power_controller PRIVATE
    Qt6::Test
    ro-control-backend
)

ro_control_register_test(test_power_controller)

# ─── Health Guard Tests ──────────────────────────────────────────────────────
qt_add_executable(test_health_guard
    test_health_guard.cpp
)

ro_control_configure_test(test_health_guard)

target_link_libraries(test_health_guard PRIVATE
    Qt6::Test
    ro-control-backend
)

ro_control_register_test(test_health_guard)

# ─── Main Window Tests ──────────────────────────────────────────────────────
qt_add_executable(test_main_window
    test_main_window.cpp
)

ro_control_configure_test(test_main_window)

target_link_libraries(test_main_window PRIVATE
    Qt6::Gui
    Qt6::Quick
    Qt6::Test
    ro-control-backend
)

ro_control_register_test(test_main_window)

# ─── D-Bus Service Tests ─────────────────────────────────────────────────────
qt_add_executable(test_dbus_service
    test_dbus_service.cpp
)

ro_control_configure_test(test_dbus_service)

target_link_libraries(test_dbus_service PRIVATE
    Qt6::DBus
    Qt6::Test
    ro-control-backend
)

ro_control_register_test(test_dbus_service)
