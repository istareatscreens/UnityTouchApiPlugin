cmake_minimum_required(VERSION 3.22.2)

set(This UnityTouchPanelApiPluginTests)
set(Project UnityTouchPanelApiPlugin)

set(Sources UnityTouchPanelApiPluginTest.cpp)

add_executable(${This} ${Sources})
target_link_libraries(${This} PUBLIC gtest_main gmock ${Project})
target_include_directories(${This} PUBLIC ${CMAKE_SOURCE_DIR}/src)

add_test(NAME ${This} COMMAND ${This})