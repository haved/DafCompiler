# The daf compiler
Daf is a statically typed systems programming language for projects of all sizes, with lots of opportunities for cool meta-programming (Eventually).
It's by no means complete, and if you take a look at the commit log you'll probably see the most basic of features added just yesterday.
I'm currently attempting to implement the compiler in OCaml using LLVM. I have never used a functional language before, but it will hopefully force me to avoid the object oriented hell that was the previous C++ attempt.
  
## Project structure
- **OCompiler** is the folder with the compiler
- **OCompilerTests** contains a bunch of daf files for testing
- buildScript.py 

## Progress
Check out my Trello board [here](https://trello.com/b/bXCZLvBz "Daf trello board").


## Building
### Dependencies
As for now I don't know what parts of my OCaml environment are needed,
but if you want to replicate it, check out the `MyOCamlAdventure.org` file in `docs/`

### Compiling
For normal building, use the build script. This will create the binary `OCompiler/dafc_main.native`
```
./buildScript.py
```

### Testing
This command will invoke the daf compiler on a bunch of daf files, run them, and expect the return code 0
```
./buildScript.py --tests
```

### Compiling your own daf
The easiest way to test out daf for yourself would be to use the testing script.
```
.buildScript.py --tests --testFolder <your folder> --test_stdout Y
```
The folder name `MyTests` is already in the .gitignore, for just this purpose.

### Installing
The binary `dafc_main.native` is self-contained and can be moved anywhere.
