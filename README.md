## The Daf Compiler
Daf is a staticly typed multi-paradigm programming language designed to make software writing easier. Without header files, and lots of opportunities for kool meta-programming.

#### Repository structure
 - Compiler: The C++ project for the compiler, using LLVM, Boost and CMake
 - CompilerTests: A folder with a bunch of daf files you'll try to compile
  - By no means advanced tests. Will only report compiler errors
 - DafLinker: The dafln python script for linking and parsing Linkfiles
 - DafParser: An old java program for parsing outdated, simple daf, and turning it into broken C++
 - Specs: contains markdown files for remembering stuff
 - Tools: contains things link an emacs mode for daf
Keep in mind that daf changes very quickly at this point, and that files may be outdated.
