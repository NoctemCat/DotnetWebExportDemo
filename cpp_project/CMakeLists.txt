cmake_minimum_required(VERSION 3.24)

set(CMAKE_CONFIGURATION_TYPES "GodotConfig" CACHE STRING "" FORCE) 

set(GODOT_PLATFORM "" CACHE STRING "")
set(EXTRA_SUFFIX "" CACHE STRING "")
set(GODOT_TYPE "static" CACHE STRING "")
set_property(CACHE GODOT_TYPE PROPERTY STRINGS static shared)
set(GODOT_TARGET "editor" CACHE STRING "")
set_property(CACHE GODOT_TARGET PROPERTY STRINGS editor template_debug template_release)
set(GODOT_ARCH "x86_64" CACHE STRING "")
set_property(CACHE GODOT_ARCH PROPERTY STRINGS x86_32 x86_64 arm32 arm64 rv64 ppc64 wasm32 loongarch64)

set(GODOT_CPP_DIR ${CMAKE_SOURCE_DIR}/../godot-cpp)
set(LIBGODOT_BUILD_DIR ${CMAKE_SOURCE_DIR}/../godot/bin)

if(GODOT_TYPE STREQUAL "static")
    set(GODOT_SUFFIX "static")
    set(CONFIG_SUFFIX "static.configure.json")
elseif(GODOT_TYPE STREQUAL "shared")
    set(GODOT_SUFFIX "shared")
    set(CONFIG_SUFFIX "shared.configure.json")
else()
    message(FATAL "Invalid godot type ${GODOT_TYPE}")
endif()
if(NOT EXTRA_SUFFIX STREQUAL "")
    set(GODOT_SUFFIX "${EXTRA_SUFFIX}.${GODOT_SUFFIX}")
    set(CONFIG_SUFFIX "${EXTRA_SUFFIX}.${CONFIG_SUFFIX}")
endif()

if(GODOT_PLATFORM STREQUAL "")
    message(FATAL "GODOT_PLATFORM must be specified")
endif()
set(PLATFORM_SUFFIX "")
if(GODOT_PLATFORM STREQUAL "linuxbsd")
    if("${GODOT_TYPE}" STREQUAL "static")
        set(PLATFORM_SUFFIX "a")
    else()
        set(PLATFORM_SUFFIX "so")
    endif()
elseif(GODOT_PLATFORM STREQUAL "windows")
    if("${GODOT_TYPE}" STREQUAL "static")
        set(PLATFORM_SUFFIX "lib")
    else()
        set(PLATFORM_SUFFIX "dll")
    endif()
elseif(GODOT_PLATFORM STREQUAL "macos")
    if("${GODOT_TYPE}" STREQUAL "static")
        set(PLATFORM_SUFFIX "a")
    else()
        set(PLATFORM_SUFFIX "dylib")
    endif()
elseif(GODOT_PLATFORM STREQUAL "web")
    if("${GODOT_TYPE}" STREQUAL "static")
        set(PLATFORM_SUFFIX "a")
    else()
        set(PLATFORM_SUFFIX "wasm")
    endif()
endif()
set(GODOT_SUFFIX "${GODOT_SUFFIX}.${PLATFORM_SUFFIX}")


file(READ "${LIBGODOT_BUILD_DIR}/libgodot.${GODOT_PLATFORM}.${GODOT_TARGET}.${GODOT_ARCH}.${CONFIG_SUFFIX}" CONFIG_JSON)
string(JSON GODOT_CFLAGS GET "${CONFIG_JSON}" CFLAGS)
string(JSON GODOT_CXXFLAGS GET "${CONFIG_JSON}" CXXFLAGS)
string(JSON GODOT_CCFLAGS GET "${CONFIG_JSON}" CCFLAGS)
string(JSON GODOT_LINKFLAGS GET "${CONFIG_JSON}" LINKFLAGS)
string(JSON GODOT_LIBS_STR GET "${CONFIG_JSON}" LIBS)
string(REPLACE " " ";" GODOT_LIBS "${GODOT_LIBS_STR}")
if(EMSCRIPTEN)
    string(JSON GODOT_EMCC_CLOSURE_ARGS GET "${CONFIG_JSON}" EMCC_CLOSURE_ARGS)
    set(ENV{EMCC_CLOSURE_ARGS} "${EMCC_CLOSURE_ARGS}")
endif()

SET(CMAKE_C_FLAGS_GODOTCONFIG "${GODOT_CFLAGS} ${GODOT_CCFLAGS}" CACHE STRING "" FORCE)
SET(CMAKE_CXX_FLAGS_GODOTCONFIG "${GODOT_CXXFLAGS} ${GODOT_CCFLAGS}" CACHE STRING "" FORCE)
SET(CMAKE_SHARED_LINKER_FLAGS_GODOTCONFIG "${GODOT_LINKFLAGS}" CACHE STRING "" FORCE)
SET(CMAKE_EXE_LINKER_FLAGS_GODOTCONFIG "${GODOT_LINKFLAGS}" CACHE STRING "" FORCE)
MARK_AS_ADVANCED(
    CMAKE_C_FLAGS_GODOTCONFIG
    CMAKE_CXX_FLAGS_GODOTCONFIG
    CMAKE_SHARED_LINKER_FLAGS_GODOTCONFIG 
    CMAKE_EXE_LINKER_FLAGS_GODOTCONFIG
)

project(cpp_sample LANGUAGES CXX)

set(GODOTCPP_SYMBOL_VISIBILITY auto CACHE STRING "" FORCE)
add_subdirectory(${GODOT_CPP_DIR} "${CMAKE_BINARY_DIR}/godot-cpp" SYSTEM)

# Remove shared library flags
set_target_properties(godot-cpp PROPERTIES COMPILE_OPTIONS "")
set_target_properties(godot-cpp PROPERTIES INTERFACE_COMPILE_OPTIONS "")
set_target_properties(godot-cpp PROPERTIES LINK_OPTIONS "")
set_target_properties(godot-cpp PROPERTIES INTERFACE_LINK_OPTIONS "")
if("${GODOT_TYPE}" STREQUAL "static")
    set_target_properties(godot-cpp PROPERTIES POSITION_INDEPENDENT_CODE OFF)
endif()

# Add godot-cpp's module path and include the exported functions.
# This is made available for documentation generation
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${godot-cpp_SOURCE_DIR}/cmake")
include(GodotCPPModule)

# The godot-cpp target has some of useful properties attached that can be retrieved like so.
get_target_property(GODOTCPP_SUFFIX godot::cpp GODOTCPP_SUFFIX)
get_target_property(GODOTCPP_PLATFORM godot::cpp GODOTCPP_PLATFORM)

set(OUTPUT_FOLDER "${CMAKE_SOURCE_DIR}/templates/${GODOT_PLATFORM}/${GODOT_TARGET}_${GODOT_TYPE}")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "$<1:${OUTPUT_FOLDER}>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "$<1:${OUTPUT_FOLDER}>")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "$<1:${OUTPUT_FOLDER}>")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
get_target_property(GODOTCPP_PLATFORM godot::cpp GODOTCPP_PLATFORM)

file(WRITE "${CMAKE_BINARY_DIR}/.gdignore" " ")

set(SOURCES src/main.cpp src/player.cpp)
find_package(Threads REQUIRED)

message(STATUS "parsed c flags: ${CMAKE_C_FLAGS_GODOTCONFIG}")
message(STATUS "parsed cxx flags: ${CMAKE_CXX_FLAGS_GODOTCONFIG}")
message(STATUS "parsed link flags: ${CMAKE_EXE_LINKER_FLAGS_GODOTCONFIG}")
add_executable(${GODOT_TARGET} ${SOURCES})
target_link_libraries(${GODOT_TARGET} godot-cpp Threads::Threads)
target_link_libraries(${GODOT_TARGET} ${GODOT_LIBS})
target_include_directories(${GODOT_TARGET} PUBLIC "${CMAKE_SOURCE_DIR}/src")
if("${GODOT_TYPE}" STREQUAL "static" OR NOT DEFINED EMSCRIPTEN)
    target_link_libraries(${GODOT_TARGET} "${LIBGODOT_BUILD_DIR}/libgodot.${GODOT_PLATFORM}.${GODOT_TARGET}.${GODOT_ARCH}.${GODOT_SUFFIX}")
else()
    target_link_options(${GODOT_TARGET} PUBLIC -sMAIN_MODULE=2 "${LIBGODOT_BUILD_DIR}/godot.side.${GODOT_PLATFORM}.${GODOT_TARGET}.${GODOT_ARCH}.${GODOT_SUFFIX}")
endif()
if(EMSCRIPTEN)
    set_target_properties(${GODOT_TARGET} PROPERTIES 
        SUFFIX ".js" 
    )

    file(REMOVE_RECURSE "${CMAKE_BINARY_DIR}/web_template")
    set(ZIP_SUFFIX "${GODOT_TYPE}.zip")
    if(NOT EXTRA_SUFFIX STREQUAL "")
        set(ZIP_SUFFIX "${EXTRA_SUFFIX}.${ZIP_SUFFIX}")
    endif()
    
    file(ARCHIVE_EXTRACT INPUT "${LIBGODOT_BUILD_DIR}/godot.${GODOT_PLATFORM}.${GODOT_TARGET}.${GODOT_ARCH}.${ZIP_SUFFIX}" 
        DESTINATION ${CMAKE_BINARY_DIR}/web_template
    )

    set(ZIP_NAME godot)
    if("${GODOT_TARGET}" STREQUAL "editor")
        set(ZIP_NAME godot.editor)
    endif()

    add_custom_command(TARGET ${GODOT_TARGET}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -DZIP_DIR="${CMAKE_BINARY_DIR}/web_template" -DGODOT_FILE="$<TARGET_FILE:${GODOT_TARGET}>" -DGODOT_NAME="${ZIP_NAME}" -DGODOT_OUTPUT="${OUTPUT_FOLDER}/${GODOT_TARGET}.zip" -P "${CMAKE_SOURCE_DIR}/cmake/PackageWeb.cmake"
    )
endif()

add_custom_target(
    copy-compile-commands ALL
    ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_BINARY_DIR}/compile_commands.json
        ${CMAKE_SOURCE_DIR}
)