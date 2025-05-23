cmake_minimum_required(VERSION 3.11)

project(Styx C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


# Build cJSON as static library
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

include(FetchContent)
FetchContent_Declare(
    cjson
    GIT_REPOSITORY https://github.com/DaveGamble/cJSON
    GIT_TAG v1.7.18
)

FetchContent_MakeAvailable(cjson)

set(BUILD_TESTS OFF CACHE BOOL "" FORCE)

file(GLOB_RECURSE DEBUG_SOURCES ${PROJECT_SOURCE_DIR}/src/*.c)
file(GLOB_RECURSE RELEASE_SOURCES ${PROJECT_SOURCE_DIR}/src/*.c)
list(REMOVE_ITEM RELEASE_SOURCES ${PROJECT_SOURCE_DIR}/src/utils/debug.c)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra -pedantic)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_executable(${PROJECT_NAME} ${DEBUG_SOURCES})
    target_compile_definitions(${PROJECT_NAME} PRIVATE DEBUG)

elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_executable(${PROJECT_NAME} ${RELEASE_SOURCES})
elseif(CMAKE_BUILD_TYPE STREQUAL "Test")
    set(BUILD_TESTS ON)
else()
    message(STATUS "Unknown build type: ${CMAKE_BUILD_TYPE}")
endif()
set(CMAKE_C_FLAGS_DEBUG "-g -fsanitize=undefined,address")
set(CMAKE_C_FLAGS_RELEASE "-O2 -Werror")
set(CMAKE_C_FLAGS_TEST "-g -fsanitize=undefined,address -fprofile-arcs -ftest-coverage" CACHE STRING "")

if(NOT BUILD_TESTS)
    target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/include)
    target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/include/setup)
    target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/include/utils)
    target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/include/run)
    target_include_directories(${PROJECT_NAME} PRIVATE ${cjson_SOURCE_DIR})
    target_link_libraries(${PROJECT_NAME} cjson)
else()
enable_testing()
    file(GLOB_RECURSE TEST_SOURCES ${PROJECT_SOURCE_DIR}/tests/*.c)
    list(APPEND TEST_SOURCES ${DEBUG_SOURCES})
    list(REMOVE_ITEM TEST_SOURCES ${PROJECT_SOURCE_DIR}/src/main.c)
    add_executable(Tests ${TEST_SOURCES})
    add_compile_options(Tests PRIVATE CMAKE_C_FLAGS_TEST)
    target_include_directories(Tests PRIVATE ${PROJECT_SOURCE_DIR}/include)
    target_include_directories(Tests PRIVATE ${PROJECT_SOURCE_DIR}/include/setup)
    target_include_directories(Tests PRIVATE ${PROJECT_SOURCE_DIR}/include/utils)
    target_include_directories(Tests PRIVATE ${PROJECT_SOURCE_DIR}/include/run)
    target_include_directories(Tests PRIVATE tests/test-include)
    target_include_directories(Tests PRIVATE ${cjson_SOURCE_DIR})
    target_compile_definitions(Tests PRIVATE TEST)
    target_compile_definitions(Tests PRIVATE DEBUG)
    target_link_libraries(Tests criterion cjson)
endif()
