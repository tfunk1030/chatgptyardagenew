cmake_minimum_required(VERSION 3.26.4)
project(GolfPhysics)

cmake_policy(SET CMP0144 NEW)
cmake_policy(SET CMP0167 NEW)

# Set compiler to MinGW GCC 14.2.0
set(CMAKE_C_COMPILER "C:/msys64/mingw64/bin/gcc.exe")
set(CMAKE_CXX_COMPILER "C:/msys64/mingw64/bin/g++.exe")

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add compiler flags for MinGW
if(MINGW)
    add_compile_options(-D_CRT_SECURE_NO_WARNINGS)
    add_compile_options(-include cstdint)
    add_compile_options(-D__STDC_FORMAT_MACROS)
    add_compile_options(-D__STDC_LIMIT_MACROS)
    # Enable all warnings and treat them as errors
    add_compile_options(-Wall -Wextra -Werror)
endif()

# Define M_PI if not defined
add_definitions(-D_USE_MATH_DEFINES)

# Boost configuration with better error handling
set(Boost_DEBUG ON)
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
set(BOOST_ROOT "C:/msys64/mingw64")
set(Boost_NO_SYSTEM_PATHS ON)

# Download and build GoogleTest
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Download and build nlohmann/json
FetchContent_Declare(
  json
  URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz
)
FetchContent_MakeAvailable(json)

# Find required packages with version requirements
find_package(Boost 1.74 REQUIRED COMPONENTS system thread)
find_package(OpenCV 4.0 REQUIRED)
# Find SQLite3
find_package(SQLite3 3.0)
if(NOT SQLite3_FOUND)
    message(STATUS "SQLite3 not found in system paths. Checking common installation directories...")
    
    # Check MSYS2 MinGW-w64 SQLite installation paths
    if(EXISTS "C:/msys64/mingw64/include/sqlite3.h")
        set(SQLite3_INCLUDE_DIR "C:/msys64/mingw64/include")
        # Use libsqlite3.dll.a as it's confirmed to exist
        set(SQLite3_LIBRARY "C:/msys64/mingw64/lib/libsqlite3.dll.a")
        message(STATUS "Found SQLite3: ${SQLite3_LIBRARY}")
    endif()

    if(SQLite3_INCLUDE_DIR AND SQLite3_LIBRARY)
        message(STATUS "Creating SQLite3 imported target")
        add_library(SQLite::SQLite3 UNKNOWN IMPORTED)
        set_target_properties(SQLite::SQLite3 PROPERTIES
            IMPORTED_LOCATION "${SQLite3_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${SQLite3_INCLUDE_DIR}")
        set(SQLite3_FOUND TRUE)
    endif()
    
    if(NOT SQLite3_INCLUDE_DIR OR NOT SQLite3_LIBRARY)
        message(FATAL_ERROR 
            "SQLite3 was not found. Please install SQLite3 or set the following variables:\n"
            "  SQLite3_INCLUDE_DIR: Directory containing sqlite3.h\n"
            "  SQLite3_LIBRARY: Path to the SQLite3 library\n"
            "Common installation path checked: C:/msys64/mingw64"
        )
    endif()
endif()
# Find CURL
find_package(CURL 7.0)
if(NOT CURL_FOUND)
    message(STATUS "CURL not found in system paths. Checking common installation directories...")
    
    # Check MSYS2 MinGW-w64 CURL installation paths
    if(EXISTS "C:/msys64/mingw64/include/curl/curl.h")
        set(CURL_INCLUDE_DIR "C:/msys64/mingw64/include")
        # Use libcurl.dll.a as it's confirmed to exist
        set(CURL_LIBRARY "C:/msys64/mingw64/lib/libcurl.dll.a")
        message(STATUS "Found CURL: ${CURL_LIBRARY}")
    endif()

    if(CURL_INCLUDE_DIR AND CURL_LIBRARY)
        message(STATUS "Creating CURL imported target")
        add_library(CURL::libcurl UNKNOWN IMPORTED)
        set_target_properties(CURL::libcurl PROPERTIES
            IMPORTED_LOCATION "${CURL_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${CURL_INCLUDE_DIR}")
        set(CURL_FOUND TRUE)
    endif()
    
    if(NOT CURL_INCLUDE_DIR OR NOT CURL_LIBRARY)
        message(FATAL_ERROR 
            "CURL was not found. Please install CURL or set the following variables:\n"
            "  CURL_INCLUDE_DIR: Directory containing curl/curl.h\n"
            "  CURL_LIBRARY: Path to the CURL library\n"
            "Common installation path checked: C:/msys64/mingw64"
        )
    endif()
endif()

if(NOT Boost_FOUND)
    message(FATAL_ERROR "Boost not found. Please set BOOST_ROOT to your Boost installation directory.")
endif()

# Add all source files
set(SOURCES
    src/main.cpp
    # Physics
    src/physics/trajectory.cpp
    src/physics/wind.cpp
    src/physics/physics.cpp
    src/physics/atmosphere.cpp
    # Data
    src/data/trackman_monitor.cpp
    src/data/gcquad_monitor.cpp
    src/data/launch_monitor_factory.cpp
    src/data/trackman_protocol.cpp
    src/data/gcquad_protocol.cpp
    src/data/baseline_data.cpp
    src/data/club_analysis.cpp
    src/data/sqlite_storage.cpp
    # ML
    src/ml/data_collector.cpp
    src/ml/player_model.cpp
    src/ml/prediction_model.cpp
    # Weather
    src/weather/weather_storage.cpp
    src/weather/weather_api.cpp
    src/weather/weather_data.cpp
)

# Create library
add_library(golf-physics ${SOURCES})

# Add include directories
target_include_directories(golf-physics 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${Boost_INCLUDE_DIRS}
        ${OpenCV_INCLUDE_DIRS}
        "C:/msys64/mingw64/include"  # Add MSYS2 MinGW-w64 include path
)

# Link dependencies
target_link_libraries(golf-physics 
    PRIVATE 
        nlohmann_json::nlohmann_json
        ${Boost_LIBRARIES}
        ${OpenCV_LIBS}
        SQLite::SQLite3
        CURL::libcurl
)

# Add test executables and link their dependencies
add_executable(physics_tests
    tests/physics/unit/trajectory_test.cpp
    tests/physics/unit/wind_test.cpp
    tests/physics/unit/atmosphere_test.cpp
    tests/physics/integration/physics_integration_test.cpp
    tests/physics/integration/wind_integration_test.cpp
)
target_link_libraries(physics_tests PRIVATE
    golf-physics
    GTest::gtest_main
)

add_executable(launch_monitor_tests
    tests/data/launch_monitor_test.cpp
    tests/data/gcquad_test.cpp
    tests/data/baseline_test.cpp
)
target_link_libraries(launch_monitor_tests PRIVATE
    golf-physics
    GTest::gtest_main
    nlohmann_json::nlohmann_json
    Threads::Threads
)

add_executable(protocol_tests
    tests/data/protocol_test.cpp
)
target_link_libraries(protocol_tests PRIVATE
    golf-physics
    GTest::gtest_main
    nlohmann_json::nlohmann_json
    ${Boost_LIBRARIES}
    ${OpenCV_LIBS}
    Threads::Threads
)

add_executable(weather_tests
    tests/weather/weather_storage_test.cpp
    tests/weather/weather_api_test.cpp
)
target_link_libraries(weather_tests PRIVATE
    golf-physics
    GTest::gtest_main
    SQLite::SQLite3
    CURL::libcurl
)

add_executable(validation_tests
    tests/validation/accuracy_test.cpp
)
target_link_libraries(validation_tests PRIVATE
    golf-physics
    GTest::gtest_main
)

# Enable testing
enable_testing()
add_test(NAME physics_tests COMMAND physics_tests)
add_test(NAME launch_monitor_tests COMMAND launch_monitor_tests)
add_test(NAME protocol_tests COMMAND protocol_tests)
add_test(NAME weather_tests COMMAND weather_tests)
add_test(NAME validation_tests COMMAND validation_tests)

# Set output directories
set_target_properties(golf-physics PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

foreach(target physics_tests launch_monitor_tests protocol_tests validation_tests weather_tests)
    set_target_properties(${target} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
endforeach()

# Install rules
install(TARGETS golf-physics
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)
