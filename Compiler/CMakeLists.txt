cmake_minimum_required (VERSION 3.4.3)
project (DafCompiler)
set(DafCompiler_VERSION_MAJOR 1)

option(EXTRA_WARNINGS "ExtraWarnings" OFF)

if(UNIX)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++17 -Weffc++")
endif()
if(EXTRA_WARNINGS)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra -pedantic")
endif()

file(GLOB MySources
    #src/*.hpp
	src/*.cpp
)

file(GLOB MySourcesParsing
    #src/parsing/*.hpp
	src/parsing/*.cpp
)

file(GLOB MySourcesParsingLexing
    #src/parsing/lexing/*.hpp
	src/parsing/lexing/*.cpp
)

file(GLOB MySourcesParsingAst
    #src/parsing/ast/*.hpp
	src/parsing/ast/*.cpp
)

file(GLOB MySourcesParsingSemantic
    #src/parsing/semantic/*.hpp
	src/parsing/semantic/*.cpp
)

file(GLOB MySourcesInfo
  #src/info/*.hpp
  src/info/*.cpp
)

#Find Boost
FIND_PACKAGE( Boost 1.40 COMPONENTS system filesystem REQUIRED )
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )

#Find LLVM
FIND_PACKAGE(LLVM 3.9.0 REQUIRED CONFIG)
MESSAGE(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
MESSAGE(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

INCLUDE_DIRECTORIES(${LLVM_INCLUDE_DIRS})
ADD_DEFINITIONS(${LLVM_DEFINITIONS})

add_executable(DafCompiler ${MySources} ${MySourcesParsing} ${MySourcesParsingLexing} ${MySourcesParsingAst} ${MySourcesParsingSemantic} ${MySourcesInfo})
target_include_directories(DafCompiler PUBLIC "${CMAKE_SOURCE_DIR}/src")

TARGET_LINK_LIBRARIES(DafCompiler ${Boost_LIBRARIES})

# Find the libraries that correspond to the LLVM components
# that we wish to use
#LLVM_MAP_COMPONENTS_TO_LIBNAMES(core) #llvm_libs irreader support   # passes target asmparser asmprinter codegen
#TARGET_LINK_LIBRARIES(DafCompiler ${llvm_libs})

TARGET_LINK_LIBRARIES(DafCompiler
  LLVMX86CodeGen
  LLVMX86AsmParser
  )

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    TARGET_LINK_LIBRARIES(DafCompiler
        LLVMXCoreCodeGen
        LLVMSystemZCodeGen
        LLVMSystemZAsmParser
        LLVMSparcCodeGen
        LLVMSparcAsmParser
        LLVMPowerPCCodeGen
        LLVMPowerPCAsmParser
        LLVMNVPTXCodeGen
        LLVMMSP430CodeGen
        LLVMMipsCodeGen
        LLVMMipsAsmParser
        LLVMHexagonCodeGen
        LLVMHexagonAsmParser
        LLVMBPFCodeGen
        LLVMARMCodeGen
        LLVMARMAsmParser
        LLVMAMDGPUCodeGen
        LLVMAMDGPUAsmParser

        LLVMAArch64CodeGen
        LLVMAArch64AsmParser
        )
else()
    target_compile_definitions(DafCompiler PRIVATE DAF_DEBUG)
endif()
