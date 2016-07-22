## Using the linker
This file contains examples of linker usage. Both the daf compiler and the daf linker are used to turn codebases into executables and libraries
This file will be using the following example folder structure:
```
project
   |----src
   |      |--**code files**
   |
   |----bin
   |      |--**object files**
   |
   |----lib
   |      |--**linked libraries**
   |           (both static and dynamic)
   |----Linkfile
```

#### Using normally
Compile every daf file:
```
daf src/me.mainFile.daf bin/ -F Linkfile
```
This will fill a folder *bin/* with `me/mainFile.o`
It will also add every object file to the Linkfile (relative to the linkfile), if the object file isn't already listed there
If libraries are used by headers imported in the project, the header often has the line: `linkfile: "-lSDL2";`
This will add -lSDL2 to the Linkfile as well.
To link the project, run `dafln -F Linkfile -o myProgram`

#### Using libraries
If using external libraries, change the following:
 - Include the library's *include/* folder when compiling, using: `-I library/include`
 - Include the library's *bin/* folder when linking, using: `-I library/bin`
 - Include the library's *lib/* folder when linking, using: `-L library/lib`
 - If dynamic libraries are used, make sure the dynamic files from *lib/* are located somewhere readable by your executable
  - Tip: Use `-rpath .` when linking to allow for dynamic library loading from the same folder, then add symlinks to the dynamic libraries
 - If the library doesn't automaticly add it's link dependencies in its headers, modify *Linkfile* yourself

#### Compiling a static library
Compile every file into *bin/* as normal. When you run dafln, run `dafln -F Linkfile -o lib/libMyLibrary.a -static` instead.
The `-static` flag makes *dafln* use the archiving tool *ar* instead of linking. Only object files are archived, and libraries give warnings.
The output should follow the naming rule `lib<name>.a`

When compiling a dynamic library, linking to libraries is not needed.