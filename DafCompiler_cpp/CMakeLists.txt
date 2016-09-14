cmake_minimum_required (VERSION 2.8)
project (DafCompiler)
set(DafCompiler_VERSION_MAJOR 1)

if(UNIX)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++17")
endif()

file(GLOB MySources
	src/*.h
	src/*.cpp
)

file(GLOB MySourcesParsing
	src/parsing/*.h
	src/parsing/*.cpp
)

file(GLOB MySourcesParsingAst
	src/parsing/ast/*.h
	src/parsing/ast/*.cpp
)

FIND_PACKAGE( Boost 1.40 COMPONENTS system filesystem REQUIRED )
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )

add_executable(DafCompiler ${MySources} ${MySourcesParsing} ${MySourcesParsingAst})

target_include_directories(DafCompiler PUBLIC "${CMAKE_SOURCE_DIR}/src")
#target_include_directories(DafCompiler PUBLIC "${CMAKE_SOURCE_DIR}/src/parsing")

TARGET_LINK_LIBRARIES(DafCompiler ${Boost_LIBRARIES} )
