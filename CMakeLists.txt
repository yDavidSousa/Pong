cmake_minimum_required(VERSION 3.2)

project(pong)

# DEPENDENCIES DIRECTORIES
set(DEPS_ROOT_DIR "${CMAKE_SOURCE_DIR}/deps")
set(DEPS_INCLUDE_DIR "${DEPS_ROOT_DIR}/include")
set(DEPS_LIBRARIES_DIR "${DEPS_ROOT_DIR}/lib/")

find_package(glfw3 3.3 REQUIRED)

add_executable(pong main.cpp)

target_include_directories(pong PUBLIC ${DEPS_INCLUDE_DIR})
target_link_directories(pong PUBLIC ${DEPS_LIBRARIES_DIR})

target_link_libraries(pong glfw "-framework Cocoa" "-framework OpenGL" "-framework IOKit")