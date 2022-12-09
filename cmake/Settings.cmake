
include(GenerateTargetTriplet)

if(NOT DEFINED PKG_TARGET_TRIPLET)
    GENERATE_TARGET_TRIPLET()
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(DEBUG_SUFFIX "debug/")
else()
    set(DEBUG_SUFFIX "")
endif()

if(NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${PKG_TARGET_TRIPLET}/${DEBUG_SUFFIX}bin")
endif()
if(NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${PKG_TARGET_TRIPLET}/${DEBUG_SUFFIX}lib")
endif()
if(NOT CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${PKG_TARGET_TRIPLET}/${DEBUG_SUFFIX}lib")
endif()
if(NOT CMAKE_BINARY_DIR)
    set(CMAKE_BINARY_DIR "${CMAKE_SOURCE_DIR}/build")
endif()

message(STATUS "CMAKE_CXX_COMPILER_ID=${CMAKE_CXX_COMPILER_ID}")

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
add_compile_options(
    "-std=c++17"
    "-Werror"
    "-Wno-deprecated-declarations"
    "-Wno-unused-result"
    "$<$<CONFIG:Debug>:-ggdb>"
    "$<$<CONFIG:Debug>:-O0>"
    "$<$<CONFIG:Release>:-O2>")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
add_compile_options(
    "-std=c++17"
    "-Werror"
    "-Wno-deprecated-declarations"
    "-Wno-unused-result"
    "$<$<CONFIG:Debug>:-ggdb>"
    "$<$<CONFIG:Debug>:-O0>"
    "$<$<CONFIG:Release>:-O2>")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
add_compile_options(
    "-std=c++17"
    "-Werror"
    "-Wno-deprecated-declarations"
    "-Wno-unused-result"
    "$<$<CONFIG:Debug>:-ggdb>"
    "$<$<CONFIG:Debug>:-O0>"
    "$<$<CONFIG:Release>:-O2>")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
add_compile_options(
    /std:c++17 /W4 /wd4324
    "$<$<CONFIG:Debug>:/ZI>"
    "$<$<CONFIG:Debug>:/Od>"
    "$<$<CONFIG:Release>:/O2>"
    "$<$<CONFIG:Release>:/WX>")
endif()