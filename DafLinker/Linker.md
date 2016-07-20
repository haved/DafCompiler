The linker is a program that takes a bunch of object files and maybe a library, and outputs a new file.
The daf linker is based on a Linkfile, containing a list of object files and libraries. The object files are located relative to the linker's search directories.
A Linkfile contains parameters written by the daf compiler while compiling each source file. Whenever a .o file is created, it is added to the Linkfile.
Daf headers for libraries can contain the keyword `linkfile "-SDL2";` or `linkfile "daf/memory.o"` to append the object file or library to the Linkfile.
The executable for the daf linker is called *dafln*, and has standard search directories defined in `/usr/share/daf/linker_search.txt` and `$HOME/.daf/linker_search.txt`.
  
NOTE: The linker_search.txt files are parsed like Linkfiles, so `-I` or `-L` is required before the directory path

##### Format and usage:
```
Usage: dafln <OPTION LIST>

List entries:
    -F <file>:      Load a linkfile
    <A file>:       Added as an object file from a search directory.
    -l<library>:    Adds a library from a library search directory.
    -I <dir>:       Add a search directory for object files
    -L <dir>:       Add a search directory for linker libraries
    -o <file>:      Set output file name
    -X <linker>:    Set the linker program used (default: ld)
    -x <arg>:       Pass a single arg to the linker
    -rpath <dir>:   Set runtime shared library search directory
    -h --help       Print this help message
```

The current working directory is also a search directory when looking for object files and libraries.
The linkfile is just the same option list.
When loading a linkfile, the relative search directory is temporarily set there, but the output will allways be relative to the working directory.
Everything behind `#` on a line is a comment.

Default behaviour:
 - All library search locations 
 - If no object files or libraries are supplied, dafln will automaticly look for and load *./Linkfile*
 - If no linker is supplied, *ld* will be used
 - If no output file is supplied, *daf.out* is used