cmake_minimum_required (VERSION 3.4.3)
project (DafCompiler)
set(DafCompiler_VERSION_MAJOR 1)

if(UNIX)
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++17 -Weffc++")
endif()

file(GLOB MySources
	src/*.hpp
	src/*.cpp
)

file(GLOB MySourcesParsing
	src/parsing/*.hpp
	src/parsing/*.cpp
)

file(GLOB MySourcesParsingLexing
	src/parsing/lexing/*.hpp
	src/parsing/lexing/*.cpp
)

file(GLOB MySourcesParsingAst
	src/parsing/ast/*.hpp
	src/parsing/ast/*.cpp
)

file(GLOB MySourcesInfo
  src/info/*.hpp
  src/info/*.cpp
)

#Find Boost
FIND_PACKAGE( Boost 1.40 COMPONENTS system filesystem REQUIRED )
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )

#Find LLVM
FIND_PACKAGE(LLVM REQUIRED CONFIG)
MESSAGE(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
MESSAGE(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

INCLUDE_DIRECTORIES(${LLVM_INCLUDE_DIRS})
ADD_DEFINITIONS(${LLVM_DEFINITIONS})

add_executable(DafCompiler ${MySources} ${MySourcesParsing} ${MySourcesParsingLexing} ${MySourcesParsingAst} ${MySourcesInfo})
target_include_directories(DafCompiler PUBLIC "${CMAKE_SOURCE_DIR}/src")

# Find the libraries that correspond to the LLVM components
# that we wish to use
LLVM_MAP_COMPONENTS_TO_LIBNAMES(llvm_libs support core irreader)
TARGET_LINK_LIBRARIES(DafCompiler ${Boost_LIBRARIES} ${llvm_libs})
