cmake_minimum_required(VERSION 3.30)

project(cpp_sample LANGUAGES CXX)


set(GODOT_CPP_DIR ${CMAKE_SOURCE_DIR}/../godot-cpp)
set(LIBGODOT_BUILD_DIR ${CMAKE_SOURCE_DIR}/../godot/bin)

add_subdirectory(${GODOT_CPP_DIR} "${CMAKE_SOURCE_DIR}/build/godot-cpp" SYSTEM)

# Add godot-cpp's module path and include the exported functions.
# This is made available for documentation generation
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${godot-cpp_SOURCE_DIR}/cmake")
include(GodotCPPModule)

# The godot-cpp target has some of useful properties attached that can be retrieved like so.
get_target_property(GODOTCPP_SUFFIX godot::cpp GODOTCPP_SUFFIX)
get_target_property(GODOTCPP_PLATFORM godot::cpp GODOTCPP_PLATFORM)


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "$<1:${CMAKE_SOURCE_DIR}/templates/${OUTPUT_FOLDER}>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "$<1:${CMAKE_SOURCE_DIR}/templates/${OUTPUT_FOLDER}>")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "$<1:${CMAKE_SOURCE_DIR}/templates/${OUTPUT_FOLDER}>")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message(STATUS "--- ${CMAKE_CXX_COMPILER}")

file(WRITE "${CMAKE_BINARY_DIR}/.gdignore" " ")

set(SOURCES src/main.cpp src/player.cpp)
add_library(cpp_sample SHARED ${SOURCES})
target_link_libraries(cpp_sample PRIVATE godot-cpp)

set_target_properties(cpp_sample PROPERTIES DEBUG_POSTFIX ".debug")
if("${OUTPUT_FOLDER}" STREQUAL "web")
    set_target_properties(cpp_sample PROPERTIES 
        SUFFIX ".wasm" 
    )

    target_compile_options(cpp_sample PRIVATE -sSIDE_MODULE=2 )
    target_link_options(cpp_sample PRIVATE -sSIDE_MODULE=2 -pthread -sEXPORTED_FUNCTIONS=_gdextension_default_init)
endif()


add_custom_target(
    copy-compile-commands ALL
    ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_BINARY_DIR}/compile_commands.json
        ${CMAKE_SOURCE_DIR}
)