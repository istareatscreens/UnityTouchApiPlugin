cmake_minimum_required(VERSION 3.22.2)

set(This UnityTouchPanelApiPlugin)

project(${This} C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Need to compile for x64 architecture or plugin wont work with unity
set(CMAKE_GENERATOR_PLATFORM x64)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/library/googletest ${CMAKE_CURRENT_BINARY_DIR}/googletest)

# Add Dependencies
set(Headers
    ${CMAKE_SOURCE_DIR}/src/UnityTouchPanelApiPlugin.h
    ${CMAKE_SOURCE_DIR}/src/ConnectionProperties.h
    ${CMAKE_SOURCE_DIR}/src/PluginWrapper.h
    ${CMAKE_SOURCE_DIR}/src/ConnectionProperties.h
)
set(Sources
    ${CMAKE_SOURCE_DIR}/src/UnityTouchPanelApiPlugin.cpp
    ${CMAKE_SOURCE_DIR}/src/PluginWrapper.cpp
)

# Note: For testing/debugging set this to STATIC, for output to unity set it to SHARED
# add_library(${This} STATIC ${Sources} ${Headers})
add_library(${This} SHARED ${Sources} ${Headers})

# Disable /clr (Common Language Runtime) otherwise Unity callbacks will not work
# Source: https://discussions.unity.com/t/communicating-c-with-c/445435/6
# set_target_properties(${This} PROPERTIES
# COMMON_LANGUAGE_RUNTIME OFF
# )
set_target_properties(UnityTouchPanelApiPlugin PROPERTIES
    LINK_FLAGS "/DEF:${CMAKE_CURRENT_SOURCE_DIR}/UnityTouchPanelApiPlugin.def"
)

set(CMAKE_VERBOSE_MAKEFILE ON)

if(MSVC)
    # Statically link MVSC runtime
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc /D_UNICODE /DUNICODE")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")

    # Need to disable /clr for unity plugin to work
    # Need to statically link MVSC runtime
    # For Debugging change flag /MT to /MD
    target_compile_options(${This} PRIVATE /clr- /MT)
endif()

# Link hidapi
target_include_directories(${This} PUBLIC ${CMAKE_SOURCE_DIR}/src)

if(WIN32)
    target_link_libraries(${This} PRIVATE setupapi)
endif()

set_target_properties(${This} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIR}"
)

# Build
set(OUTPUT_DIR "${CMAKE_SOURCE_DIR}/../../Runtime/Plugins")
set_target_properties(${This} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIR}"
)

# Copy library to unity and clean it up
# Note for testing comment this out (Unity will often lock the output file)
add_custom_command(TARGET ${This} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E remove "${OUTPUT_DIR}/${This}.dll"
    COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:${This}>" "${OUTPUT_DIR}/${This}.dll"
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${OUTPUT_DIR}/Release"
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${OUTPUT_DIR}/Debug"
    COMMENT "Copying AdxTouchPanelApiPlugin.dll to Project Plugins directory and replacing the existing file and removing Debug"
)

# Testing
include(Dart)
enable_testing()
add_subdirectory(test)