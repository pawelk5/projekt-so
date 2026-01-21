cmake_minimum_required(VERSION 3.24)
project(park_rozrywki VERSION 1.0)

include(CTest)
enable_testing()

add_subdirectory("dependencies")
add_subdirectory("lib")
add_subdirectory("app")