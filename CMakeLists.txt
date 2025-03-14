cmake_minimum_required(VERSION 3.10)
project(BlackScholesOptionTrading)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(Qt5 COMPONENTS Core Widgets Charts REQUIRED)

# Include directories
include_directories(include)
include_directories(${CURL_INCLUDE_DIRS})

# Add source files
add_executable(option_trading 
    src/main.cpp
    src/option.cpp
    src/black_scholes.cpp
    src/market_data.cpp
    src/paper_trading.cpp
    src/alpha_vantage_client.cpp
    src/black_scholes_greeks.cpp
)

# Add source files for the GUI application
# Add source files for the GUI application
add_executable(options_calculator_gui
    src/gui_main.cpp
    src/options_calculator_gui.cpp
    src/option.cpp
    src/black_scholes.cpp
    src/market_data.cpp
    src/black_scholes_greeks.cpp
)

# Set Qt properties for the GUI application
set_target_properties(options_calculator_gui PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)

# Link libraries for the GUI application
target_link_libraries(options_calculator_gui
    ${CURL_LIBRARIES}
    nlohmann_json::nlohmann_json
    Qt5::Core
    Qt5::Widgets
    Qt5::Charts
    m
)

add_custom_command(TARGET options_calculator_gui POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_SOURCE_DIR}/.env
    ${CMAKE_BINARY_DIR}/.env
)

# Link libraries
target_link_libraries(option_trading 
    ${CURL_LIBRARIES}
    nlohmann_json::nlohmann_json
    m
)

add_custom_command(TARGET option_trading POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_SOURCE_DIR}/.env
    ${CMAKE_BINARY_DIR}/.env
)