
cmake_minimum_required(VERSION 3.18)

set(CMAKE_C_STANDARD 11)

project(thumb-test-gba C)

gba_add_library_subdirectory(rom seven)

add_executable(thumb-test main.c)
set_target_properties(thumb-test PROPERTIES SUFFIX ".elf")

target_compile_options(thumb-test PRIVATE -ffunction-sections -fdata-sections -mthumb)

target_link_libraries(thumb-test PRIVATE rom seven)

gba_target_objcopy(thumb-test FIX_HEADER)
