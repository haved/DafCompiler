cmake_minimum_required (VERSION 2.8)
project (DafCompiler)
set(DafCompiler_VERSION_MAJOR 1)

if(UNIX)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11")
endif()

file(GLOB MySources
	src/*.h
	src/*.cpp
)

add_executable(DafCompiler ${MySources})

target_include_directories(DafCompiler PUBLIC "${CMAKE_SOURCE_DIR}/src")
