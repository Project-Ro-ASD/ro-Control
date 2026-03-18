find_package(Qt6 REQUIRED COMPONENTS Test)

set(RO_CONTROL_TEST_COMPILE_DEFINITIONS
    RO_CONTROL_POLICY_ID="${RO_CONTROL_POLICY_ID}"
    RO_CONTROL_HELPER_BUILD_PATH="${RO_CONTROL_HELPER_BUILD_PATH}"
    RO_CONTROL_HELPER_INSTALL_PATH="${RO_CONTROL_HELPER_INSTALL_PATH}"
)

# ─── Detector Tests ──────────────────────────────────────────────────────────
qt_add_executable(test_detector
    test_detector.cpp
    ${CMAKE_SOURCE_DIR}/src/backend/nvidia/detector.cpp
    ${CMAKE_SOURCE_DIR}/src/backend/system/commandrunner.cpp
)

target_include_directories(test_detector PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/backend
)

target_compile_definitions(test_detector PRIVATE
    ${RO_CONTROL_TEST_COMPILE_DEFINITIONS}
)

target_link_libraries(test_detector PRIVATE
    Qt6::Core
    Qt6::Test
)

add_test(NAME test_detector COMMAND test_detector)

# ─── Updater/Version Parser Tests ────────────────────────────────────────────
qt_add_executable(test_updater
    test_updater.cpp
    ${CMAKE_SOURCE_DIR}/src/backend/nvidia/versionparser.cpp
)

target_include_directories(test_updater PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/backend
)

target_link_libraries(test_updater PRIVATE
    Qt6::Core
    Qt6::Test
)

add_test(NAME test_updater COMMAND test_updater)

# ─── Monitor Tests ───────────────────────────────────────────────────────────
qt_add_executable(test_monitor
    test_monitor.cpp
    ${CMAKE_SOURCE_DIR}/src/backend/monitor/cpumonitor.cpp
    ${CMAKE_SOURCE_DIR}/src/backend/monitor/gpumonitor.cpp
    ${CMAKE_SOURCE_DIR}/src/backend/monitor/rammonitor.cpp
    ${CMAKE_SOURCE_DIR}/src/backend/system/commandrunner.cpp
)

target_include_directories(test_monitor PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/backend
)

target_compile_definitions(test_monitor PRIVATE
    ${RO_CONTROL_TEST_COMPILE_DEFINITIONS}
)

target_link_libraries(test_monitor PRIVATE
    Qt6::Core
    Qt6::Test
)

add_test(NAME test_monitor COMMAND test_monitor)

# ─── System Integration Tests ───────────────────────────────────────────────
qt_add_executable(test_system_integration
    test_system_integration.cpp
    ${CMAKE_SOURCE_DIR}/src/backend/system/commandrunner.cpp
    ${CMAKE_SOURCE_DIR}/src/backend/system/dnfmanager.cpp
    ${CMAKE_SOURCE_DIR}/src/backend/system/polkit.cpp
)

target_include_directories(test_system_integration PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/backend
)

target_compile_definitions(test_system_integration PRIVATE
    ${RO_CONTROL_TEST_COMPILE_DEFINITIONS}
)

target_link_libraries(test_system_integration PRIVATE
    Qt6::Core
    Qt6::Test
)

add_test(NAME test_system_integration COMMAND test_system_integration)

# ─── Metadata Tests ──────────────────────────────────────────────────────────
qt_add_executable(test_metadata
    test_metadata.cpp
)

target_compile_definitions(test_metadata PRIVATE
    ${RO_CONTROL_TEST_COMPILE_DEFINITIONS}
    RO_CONTROL_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
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
    ${CMAKE_SOURCE_DIR}/src/backend/nvidia/detector.cpp
    ${CMAKE_SOURCE_DIR}/src/backend/nvidia/updater.cpp
    ${CMAKE_SOURCE_DIR}/src/backend/nvidia/versionparser.cpp
    ${CMAKE_SOURCE_DIR}/src/backend/monitor/cpumonitor.cpp
    ${CMAKE_SOURCE_DIR}/src/backend/monitor/gpumonitor.cpp
    ${CMAKE_SOURCE_DIR}/src/backend/monitor/rammonitor.cpp
    ${CMAKE_SOURCE_DIR}/src/backend/system/commandrunner.cpp
)

target_include_directories(test_cli PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/backend
)

target_compile_definitions(test_cli PRIVATE
    ${RO_CONTROL_TEST_COMPILE_DEFINITIONS}
)

target_link_libraries(test_cli PRIVATE
    Qt6::Core
    Qt6::Test
)

add_test(NAME test_cli COMMAND test_cli)
