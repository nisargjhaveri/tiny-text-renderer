cmake_minimum_required(VERSION 3.10)
project(tiny-text-renderer)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS True)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

if (APPLE)
  add_link_options("LINKER:-object_path_lto,${CMAKE_BINARY_DIR}/$<TARGET_PROPERTY:NAME>.lto")
  add_link_options("LINKER:-cache_path_lto,${CMAKE_BINARY_DIR}/LTOCache")
endif ()

add_subdirectory(src)

add_executable(tiny-text-renderer-demo 
    main.cpp
)

target_link_libraries(tiny-text-renderer-demo
    tiny-text-renderer
)
