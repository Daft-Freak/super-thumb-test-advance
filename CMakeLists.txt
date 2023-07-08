cmake_minimum_required(VERSION 3.18)

project(thumb-test)


if(GBA)
    add_subdirectory(gba)
else()
    add_subdirectory(32blit)
endif()