cmake_minimum_required(VERSION 3.0.0)
project(mandelbrot_set VERSION 0.1.0)

include(CTest)
enable_testing()

include_directories(include)
find_package(SFML 3 REQUIRED COMPONENTS System Window Graphics)
#link_directories(lib)

file(GLOB SOURCES "src/*.cpp")


add_executable(${PROJECT_NAME} ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-O3")

target_link_libraries(${PROJECT_NAME} PRIVATE SFML::System SFML::Window SFML::Graphics boost_thread)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
