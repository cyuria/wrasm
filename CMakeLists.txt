cmake_minimum_required(VERSION 3.25)

project(wrasm VERSION 0.0.1 LANGUAGES C)

file(GLOB_RECURSE sources src/*.c)
add_executable(${PROJECT_NAME} ${sources})

set_target_properties(${PROJECT_NAME} PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

target_include_directories(${PROJECT_NAME} PRIVATE h)
target_link_libraries(${PROJECT_NAME} m)

