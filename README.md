## The Daf Compiler
Daf is a staticly typed multi-paradigm programming language designed to make software writing easier. Without header files, and lots of opportunities for kool meta-programming.

### Repository structure
 - Compiler: The C++ project for the compiler, using LLVM, Boost and CMake
 - CompilerTests: A folder with a bunch of daf files you'll try to compile
  - By no means advanced tests. Will only report compiler errors
 - DafLinker: The dafln python script for linking and parsing Linkfiles
 - DafParser: An old java program for parsing outdated, simple daf, and turning it into broken C++
 - Specs: contains markdown files for remembering stuff
 - Tools: contains things link an emacs mode for daf
Keep in mind that daf changes very quickly at this point, and that files may be outdated. Last cleaned 2017-12-23


### Building
**Dependencies**
- Boost filesystem
- LLVM 5.0.x

If you want to build a debug build of the DafCompiler (extra output), you'll need a Debug build of llvm.
Start building llvm from source, following their instructions, but supply the `-DCMAKE_BUILD_TARGET=Debug` to cmake.
See the document Specs/InstallingLLVMFromSource.org for details.
**NOTE:** If you don't need a debug build of the DafCompiler, just invoke the build script with --release

The script `buildScript.py` lets you build and test run Debug or Release builds of the compiler.
It invokes cmake and make for you with sensible parameters.

#### Debug (Requires Debug LLVM build) and test
```
./buildScript.py --tests
```

#### Release and test
```
./buildScript.py --release --tests
```

Looking through the `--help` of the buildScript gives you extra options, e.g. filtering tests to only compile your file

### Compiling your own daf
The easiest way of running some daf of your own is to just write a file as a test, and tell the build script to test it and print all output:
```
./buildScript.py --release --tests --testFolder <yourFolder> --testFilter <nameOfYourFile> --outputTesting
```

