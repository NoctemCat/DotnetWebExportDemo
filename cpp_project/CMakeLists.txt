cmake_minimum_required(VERSION 3.30)


project(cpp_sample)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/templates)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/templates)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/templates)
set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Threads REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -fno-exceptions")

set(GODOT_CPP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../godot-cpp)
set(LIBGODOT_BUILD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../godot/bin)

add_subdirectory(${GODOT_CPP_DIR} "${CMAKE_SOURCE_DIR}/build/godot-cpp" SYSTEM)

# Add godot-cpp's module path and include the exported functions.
# This is made available for documentation generation
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${godot-cpp_SOURCE_DIR}/cmake")
include(GodotCPPModule)

# The godot-cpp target has some of useful properties attached that can be retrieved like so.
get_target_property(GODOTCPP_SUFFIX godot::cpp GODOTCPP_SUFFIX)
get_target_property(GODOTCPP_PLATFORM godot::cpp GODOTCPP_PLATFORM)

set(SOURCES src/main.cpp src/player.cpp)

add_executable(editor ${SOURCES})
target_link_libraries(editor rt pthread dl godot-cpp Threads::Threads)
target_include_directories(editor PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(editor "${LIBGODOT_BUILD_DIR}/libgodot.linuxbsd.editor.x86_64.llvm.static.a")

add_executable(template_debug ${SOURCES})
target_link_libraries(template_debug rt pthread dl godot-cpp Threads::Threads)
target_include_directories(template_debug PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(template_debug "${LIBGODOT_BUILD_DIR}/libgodot.linuxbsd.template_debug.x86_64.llvm.static.a")

add_executable(template_release ${SOURCES})
target_link_libraries(template_release rt pthread dl godot-cpp Threads::Threads)
target_include_directories(template_release PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(template_release "${LIBGODOT_BUILD_DIR}/libgodot.linuxbsd.template_release.x86_64.llvm.static.a")
