The linker is a program that takes a bunch of object files and maybe a library, and outputs a new file.
The daf linker is based on a Linkfile, containing a list of object files and libraries. The object files are located relative to the linker's search directories.
A Linkfile contains parameters written by the daf compiler while compiling each source file. Whenever a .o file is created, it is added to the Linkfile.
Daf headers for libraries can contain the keyword `linkfile "-SDL2";` or `linkfile "daf/memory.o"` to append the object file or library to the Linkfile.
The executable for the daf linker is called *dafln*, and has standard search directories defined in */usr/share/daf/linker_search.txt* and *$HOME/.daf/linker_search.txt*.

##### Format and usage:
```
Usage: dafln <OPTION LIST>

Lits entries:
    -L <file>:   Load a linkfile
    <A file>:    Added as object file, relative to working directory or a search directory.
    -l<library>: Adds a library sent to the linker.
    -I <dir>:    Add a serch directory for object files
    -o <file>:   Set output file name
    -X <linker>: Set the linker program used
    -x <option>: Pass an option to the linker
    -h --help    Print this help message
 ```

The linkfile is just the same option list.
When loading a linkfile, the relative search directory is temporarily set there, but the output will allways be relative to the working directory.
Libraries are not

 If no object files or libraries are supplied, dafln will automaticly look for and load *./Linkfile*
 If no linker is supplied, *ln* will be used
 If no output file is supplied, *daf.out* is used