cmake_minimum_required(VERSION 3.15)
project(Platform.Delegates)

add_library(Platform.Delegates INTERFACE)
target_include_directories(Platform.Delegates INTERFACE Platform.Delegates)

if(${LINKS_PLATFORM_TESTS})
    add_executable(Platform.Delegates.Tests.Console Platform.Delegates.Tests.Console/Console.cpp)
    target_link_libraries(Platform.Delegates.Tests.Console PUBLIC Platform.Delegates)
    set_target_properties(Platform.Delegates.Tests.Console PROPERTIES CXX_STANDARD 20)
endif()
