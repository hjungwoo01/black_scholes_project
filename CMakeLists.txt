cmake_minimum_required(VERSION 3.14)
project(BlackScholesOptionTrading LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# nlohmann_json: fetch if not found (v3.12+ has CMake 3.5+ compatible config)
find_package(nlohmann_json 3.2.0 QUIET)
if(NOT nlohmann_json_FOUND)
  include(FetchContent)
  FetchContent_Declare(json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.12.0
  )
  FetchContent_MakeAvailable(json)
endif()

find_package(CURL REQUIRED)

# Qt5 optional (for GUI only)
find_package(Qt5 QUIET COMPONENTS Core Widgets Charts)
set(BUILD_GUI ${Qt5_FOUND})

# Include directories
include_directories(include)
include_directories(${CURL_INCLUDE_DIRS})

# Math library (Unix/macOS)
if(UNIX OR APPLE)
  set(MATH_LIB m)
else()
  set(MATH_LIB "")
endif()

# CLI executable (always built)
add_executable(option_trading
  src/main.cpp
  src/option.cpp
  src/black_scholes.cpp
  src/market_data.cpp
  src/paper_trading.cpp
  src/alpha_vantage_client.cpp
  src/black_scholes_greeks.cpp
)
target_link_libraries(option_trading PRIVATE
  CURL::libcurl
  nlohmann_json::nlohmann_json
  ${MATH_LIB}
)
add_custom_command(TARGET option_trading POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
  ${CMAKE_SOURCE_DIR}/.env
  $<TARGET_FILE_DIR:option_trading>/.env
)

# GUI executable (only if Qt5 found)
if(BUILD_GUI)
  add_executable(options_calculator_gui
    src/gui_main.cpp
    src/options_calculator_gui.cpp
    include/options_calculator_gui.h
    src/option.cpp
    src/black_scholes.cpp
    src/market_data.cpp
    src/alpha_vantage_client.cpp
    src/black_scholes_greeks.cpp
  )
  set_target_properties(options_calculator_gui PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
  )
  target_include_directories(options_calculator_gui PRIVATE ${CMAKE_SOURCE_DIR}/include)
  target_link_libraries(options_calculator_gui PRIVATE
    CURL::libcurl
    nlohmann_json::nlohmann_json
    Qt5::Core
    Qt5::Widgets
    Qt5::Charts
    ${MATH_LIB}
  )
  add_custom_command(TARGET options_calculator_gui POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_SOURCE_DIR}/.env
    $<TARGET_FILE_DIR:options_calculator_gui>/.env
  )
  message(STATUS "GUI (options_calculator_gui) enabled: Qt5 found")
else()
  message(STATUS "GUI disabled: Qt5 not found. Build option_trading for CLI.")
endif()
