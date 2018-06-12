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
For building OCaml, `oasis` is used, but the generated files are checked in to the git repository.
See `docs/MyOCamlAdventure.org` in case you want to re-do the oasis things.
  
For normal building, use the build script
```
./buildScript.py
```

### Testing

### Compiling your own daf
The easiest way to write and run daf would be making your own tests
For anything more serious read `dafc --help`

