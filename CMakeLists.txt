cmake_minimum_required(VERSION 3.10)
project(tiny-text-renderer)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS True)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(src)

add_executable(tiny-text-renderer-demo 
    main.cpp
)

target_link_libraries(tiny-text-renderer-demo
    tiny-text-renderer
)
