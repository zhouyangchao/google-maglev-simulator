cmake_minimum_required(VERSION 3.10)
project(google-maglev-simulator C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(READLINE REQUIRED readline)

# Create the main executable
add_executable(maglev-simulator
    src/main.c
    src/maglev.c
    src/node.c
    src/hash.c
)

target_include_directories(maglev-simulator PRIVATE include ${READLINE_INCLUDE_DIRS})
target_link_libraries(maglev-simulator ${READLINE_LIBRARIES})
target_link_directories(maglev-simulator PRIVATE ${READLINE_LIBRARY_DIRS})
target_compile_options(maglev-simulator PRIVATE ${READLINE_CFLAGS_OTHER})

# Enable debug symbols for development
set(CMAKE_BUILD_TYPE Debug)