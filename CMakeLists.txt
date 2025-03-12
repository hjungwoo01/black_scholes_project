cmake_minimum_required(VERSION 3.10)
project(BlackScholesOptionTrading)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)

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