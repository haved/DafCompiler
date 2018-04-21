cmake_minimum_required (VERSION 3.4.3)
project (DafCompiler)
set(DafCompiler_VERSION_MAJOR 1)

option(DAF_DEBUG_MACRO "Enable #define DAF_DEBUG_MACRO (Overridden if CMAKE_BUILD_TYPE=Debug)" OFF)
option(DAF_TARGET_ONLY_x86 "Ignore most LLVM targets" OFF)
option(DAF_SHARED_LINKING_LLVM "link with libLLVM.so instead of a bunch of .a files" ON)

# User specified strings:
# DAF_EXTRA_CXX_FLAGS:String //Default: ""
# DAF_EXTRA_LINK_FLAGS:String //Default: ""
#
# You might want to play with these:
# CMAKE_CXX_COMPILER
# CMAKE_LINKER
# CMAKE_BUILD_TYPE:String #Not used by the buildScript because it does extra things to asserts and whatnot

IF(CMAKE_BUILD_TYPE MATCHES "Rel") #if build type starts with Rel
    MESSAGE(WARNING "Using CMAKE_BUILD_TYPE to build Release removes asserts. Will give many warnings")
ENDIF()
IF(CMAKE_BUILD_TYPE MATCHES "Debug")
  MESSAGE(WARNING "You are using CMAKE_BUILD_TYPE to choose Debug, so I'll enable DAF_DEBUG_MACRO")
  SET(DAF_DEBUG_MACRO ON)
ENDIF()


SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${DAF_EXTRA_CXX_FLAGS}") #Examples: -Wall -Weffc++ -Wextra -pedantic -g -O2
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${DAF_EXTRA_LINK_FLAGS}")
#if(UNIX)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
#endif()

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
FIND_PACKAGE(LLVM 6.0 REQUIRED CONFIG)
MESSAGE(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
MESSAGE(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

INCLUDE_DIRECTORIES(${LLVM_INCLUDE_DIRS})
ADD_DEFINITIONS(${LLVM_DEFINITIONS})

add_executable(DafCompiler ${MySources} ${MySourcesParsing} ${MySourcesParsingLexing} ${MySourcesParsingAst} ${MySourcesParsingSemantic} ${MySourcesInfo})
target_include_directories(DafCompiler PUBLIC "${CMAKE_SOURCE_DIR}/src")

MESSAGE(INFO " DAF_DEBUG_MACRO=${DAF_DEBUG_MACRO}")
if(DAF_DEBUG_MACRO)
  if(NOT LLVM_BUILD_TYPE STREQUAL "Debug")
	MESSAGE(FATAL_ERROR "Can't build a Debug build of the DafCompiler without a Debug build of LLVM, perhaps use the --release option")
  endif()
  target_compile_definitions(DafCompiler PRIVATE DAF_DEBUG)
endif()

TARGET_LINK_LIBRARIES(DafCompiler ${Boost_LIBRARIES})

if(DAF_SHARED_LINKING_LLVM)
	TARGET_LINK_LIBRARIES(DafCompiler LLVM) #One shared library
else()
	TARGET_LINK_LIBRARIES(DafCompiler #The strictly necessary llvm linking on x86
  	LLVMX86CodeGen
  	LLVMX86AsmParser
  	)
endif()

MESSAGE(INFO " DAF_TARGET_ONLY_x86=${DAF_TARGET_ONLY_x86}")
if(DAF_TARGET_ONLY_x86)
  target_compile_definitions(DafCompiler PRIVATE DAF_TARGET_ONLY_x86)
elseif(NOT DAF_SHARED_LINK_LLVM)
  TARGET_LINK_LIBRARIES(DafCompiler #Warning, this will make the binary almost a gigabyte large
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
	LLVMLanaiAsmParser
	LLVMLanaiCodeGen
    )
endif()

#LLVM_MAP_COMPONENTS_TO_LIBNAMES(core irreader support passes target asmparser asmprinter codegen)
#TARGET_LINK_LIBRARIES(DafCompiler "${llvm_libs}")
#TARGET_LINK_LIBRARIES(DafCompiler LLVM-5.0)
