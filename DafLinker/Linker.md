The linker is a program that takes a bunch of object files and maybe a library, and outputs a new file.
The daf linker is based on a Linkfile, containing a list of object files and libraries. The object files are located relative to the linker's search directories.
A Linkfile contains parameters written by the daf compiler while compiling each source file. Whenever a .o file is created, it is added to the Linkfile.
Daf headers for libraries can contain the keyword `linkfile "-lSDL2";` or `linkfile "daf/memory.o"` to append the object file or library to the Linkfile.
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
    -static:        Compile a static library (using ar, and only object files)
    -shared:        Compile a shared library with all the files and libraries
    -soname <name>: Specify a soname for the shared library (only)
    -h --help       Print this help message
```

The current working directory is also a search directory when looking for object files and libraries.
Linkfiles are parsed just like command options, but also change the working directory to the directory of the Linkfile when loaded.
Everything behind `#` on a line in a linkfile is a comment.
When compiling a static library, the output should be `lib*.a`. Only the object files are packed together, even if libraries are passed.
When compiling a shared library, the output should be `lib*.so.<version>`. Both object files and libraries are linked together to form the shared library.
The shared library has a soname to indicate backwards compatability. If a newer version has the same soname as the previous, the new version can safely replace the old one
The format for a soname is `lib*.so.<so-version>`. 
For example, a shared library might be compiled to the filename `libHYV.so.1.2.1` but have the soname `libHYV.so.1.1` 
to maintain backwards compatability to an earlier file with the same soname.

Default behaviour:
 - All library search locations 
 - If no object files or libraries are supplied, dafln will automaticly look for and load *./Linkfile*
 - If no linker is supplied, *ld* will be used
 - If no output file is supplied, *daf.out* is used
 - If a static library is compiled, libraries and an rpath will give warnings
 - If a soname is passed while not compiling a shared library, errors are given