cmake_minimum_required(VERSION 4.2) # Use a modern version
project(skymount VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23) # Set the C++ standard to C++17
set(CMAKE_CXX_STANDARD_REQUIRED True) # Enforce the standard

file(GLOB SOURCE_FILES "src/**.cpp")
file(GLOB HEADER_FILES "include/**.h")
add_executable(skymount ${SOURCE_FILES} ${HEADER_FILES})
include_directories("include/")
find_package(reflectcpp REQUIRED)
target_link_libraries(skymount PRIVATE reflectcpp::reflectcpp)
