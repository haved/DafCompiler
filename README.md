## The Daf Compiler
Daf is a statically typed systems programming language for projects of all sizes, with lots of opportunities for cool meta-programming (Eventually).
It's by no means complete, and if you take a look at the commit log you'll probably see the most basic of features added just yesterday.
I should probably have some kind of example code to show, but all previous attempts became outdated very quickly. Its grammar is as volatile as my memory of it.
Also, expect the compiler to assert false here and there.

### Repository structure
 - Compiler: The C++ project for the compiler, using LLVM, Boost and CMake
 - CompilerTests: A folder with a bunch of daf files you'll try to compile
   - By no means advanced tests. Will only report compiler errors
 - Specs: contains org mode files for remembering stuff
 - Tools: contains things like an emacs mode for daf

### Progress
Check out my Trello board [here](https://trello.com/b/bXCZLvBz "Daf trello board").

### Building
**Dependencies**
- Boost filesystem
- LLVM 5.0.x

If you want to build a debug build of the DafCompiler (extra output), you'll need a Debug build of llvm.
Start building llvm from source, following their instructions, but supply the `-DCMAKE_BUILD_TARGET=Debug` to cmake.
See the document Specs/InstallingLLVMFromSource.org for details.  
**NOTE:** If you don't need a debug build of the DafCompiler, just invoke the build script with --release

##### Using buildScript.py
The script `buildScript.py` lets you build and test run Debug or Release builds of the compiler.
It invokes cmake and make for you with sensible default parameters, as long as you call the script from the root folder.
##### Using an IDE
The `CMakeLists.txt` file in `Compiler/` can be opened by your favorite IDE. Here you can build both Debug and Release builds, with similar results to the build script.
Do keep in mind that the build script doesn't use `CMAKE_BUILD_TYPE`, so whatever default compiler flags cmake has will be used instead.
This might include ignoring assert macros in Release builds, which will give unused parameter warnings, besides being really unsafe.  
**NOTE:** `CMakeLists.txt` is set up to use the Debug build of LLVM if you pass `CMAKE_BUILD_TYPE=Debug`, requiring a debug build of LLVM.  
**Tip:** Want to build with IDE but still use the buildScript? pass `--ignoreCompile --buildDir <folder with binary>`

#### Debug (Requires Debug LLVM build) and test all normal tests
```
./buildScript.py --tests
```

#### Release and test one file
```
./buildScript.py --release --tests --testFilter TestFile.daf
```

Looking through the `--help` of the build script gives you extra options, e.g. passing flags to make to use more threads.

### Compiling your own daf
The easiest way of running some daf of your own is to just write a file as a test, and tell the build script to test it and print all output (both from DafCompiler and the program you wrote).
MyTests/ is already in the gitignore, so feel free to make such a folder with some daf files and invoke:
```
./buildScript.py --release --tests --testFolder MyTests/ --testFilter <nameOfYourFile> --outputTesting
```

