set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

set(CMAKE_CXX_FLAGS "-Oz")

add_library(tiny-text-renderer
    harfbuzz/src/harfbuzz.cc
    harfbuzz_schrift.c
    schrift.c
)

target_include_directories(tiny-text-renderer
    PUBLIC
    harfbuzz/src
    .
)

add_definitions(-DHB_TINY)
add_definitions(-DHB_CONFIG_OVERRIDE_H="harfbuzz-config-override.h")
