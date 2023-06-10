cmake_minimum_required(VERSION 3.15)
set(LINKS_PLATFORM_TESTS ON CACHE BOOL "Whether to compile tests")

project(Platform.Delegates CXX)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR})
find_package(GTest REQUIRED)

message("GTest found: ${GTest_FOUND}")
message("GTest includes: ${GTEST_INCLUDE_DIRS}")
message("GTest libraries: ${GTEST_BOTH_LIBRARIES}")


add_library(Platform.Delegates INTERFACE)
target_include_directories(Platform.Delegates INTERFACE Platform.Delegates)

if(${LINKS_PLATFORM_TESTS})
    add_executable(Platform.Delegates.Tests1 Platform.Delegates.Tests/AllTests.cpp)
    set_target_properties(Platform.Delegates.Tests1 PROPERTIES CXX_STANDARD 20)
    target_link_libraries(Platform.Delegates.Tests1 PRIVATE GTest::gtest)
    target_link_libraries(Platform.Delegates.Tests1 PRIVATE GTest::gtest_main)
    target_link_libraries(Platform.Delegates.Tests1 PRIVATE Platform.Delegates)
endif()

