cmake_minimum_required(VERSION 3.15)

project(WeatherApp VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# If you use vcpkg, pass the toolchain file from command line:
# cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg.cmake

find_package(Boost REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(OpenSSL REQUIRED)

add_executable(
    WeatherApp
    main.cpp
    WeatherClient.cpp
    WeatherParser.cpp
    Database.cpp
    AuthService.cpp
    SessionManager.cpp
    ConsoleUi.cpp
)

target_link_libraries(WeatherApp PRIVATE
    Boost::headers
    nlohmann_json::nlohmann_json
    SQLite3::SQLite3
    OpenSSL::SSL
    OpenSSL::Crypto
)