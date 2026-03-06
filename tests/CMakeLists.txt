find_package(Qt6 REQUIRED COMPONENTS Test)

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

target_link_libraries(test_detector PRIVATE
    Qt6::Core
    Qt6::Test
)

add_test(NAME test_detector COMMAND test_detector)

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

target_link_libraries(test_monitor PRIVATE
    Qt6::Core
    Qt6::Test
)

add_test(NAME test_monitor COMMAND test_monitor)
