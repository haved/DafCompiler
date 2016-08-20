## The Daf Compiler
Daf is a staticly typed multi-paradigm programming language designed to make software writing easier. Without header files, and lots of oppertunities for kool meta-programming.

#### Repository structure
 - DafLinker: The dafln python script for linking and parsing Linkfiles
 - DafParser: A java program for parsing old, simple daf, and turning it into C++
 - DafCompiler: A rust project using llvm to compile daf into object files
 - TestCode: A folder for daf examples. Might be old.
 - Specs: contains markdown files for remembering stuff
Keep in mind that daf changes very quickly at this point, and that both TestCode and Specs might be outdated.

#### Example
Find an example from 2016-08-20 [here](https://github.com/haved/DafCompiler/blob/master/Example.daf "Daf example file")