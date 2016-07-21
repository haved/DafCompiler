## Using the linker
This file contains examples of linker usage. Both the daf compiler and the daf linker are used to turn codebases into executables and libraries
This file will be using the following example folder structure:
```
project
   |----code
   |      |--**code files**
   |
   |----Linkfile
```

#### Using normally
Compile every daf file:
```
daf code/file.daf bin
```
This will fill a new folder *bin* with *code/file.o*  
It will also add every object file to the Linkfile (relative to the linkfile), if the file isn't already there
If libraries are used by headers imported in the project, the header often has the line: `linkfile: "-lSDL2";`
This will add -lSDL2 to the Linkfile as well