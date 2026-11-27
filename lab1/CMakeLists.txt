cmake_minimum_required(VERSION 3.20)
project(lab1_sets_and_polynomials LANGUAGES CXX)

include(CTest)
include(FetchContent)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/utf-8)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

add_library(lab1_core
    src/polynomial.cpp
    src/set.cpp
)
target_include_directories(lab1_core PUBLIC include)

if(MSVC)
    target_compile_options(lab1_core PRIVATE /W4 /permissive-)
else()
    target_compile_options(lab1_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(lab1_cli src/main.cpp)
target_link_libraries(lab1_cli PRIVATE lab1_core)

install(TARGETS lab1_cli RUNTIME DESTINATION bin)

if(BUILD_TESTING)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.8.1
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(Catch2)

    add_executable(lab1_tests tests/test_main.cpp)
    target_link_libraries(lab1_tests PRIVATE lab1_core Catch2::Catch2WithMain)
    add_test(NAME lab1_tests COMMAND lab1_tests)

    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(lab1_core PRIVATE --coverage -O0)
        target_link_options(lab1_core INTERFACE --coverage)
        target_compile_options(lab1_tests PRIVATE --coverage -O0)
        target_link_options(lab1_tests PRIVATE --coverage)
    endif()
endif()
