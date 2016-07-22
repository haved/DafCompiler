The linker is a program that takes a bunch of object files and maybe a library, and outputs a new file.
The daf linker is based on a Linkfile, containing a list of object files and libraries. The object files are located relative to the linker's search directories.
A Linkfile contains parameters written by the daf compiler while compiling each source file. Whenever a .o file is created, it is added to the Linkfile.
Daf headers for libraries can contain the keyword `linkfile "-lSDL2";` or `linkfile "daf/memory.o"` to append the object file or library to the Linkfile.
The executable for the daf linker is called *dafln*, and has standard search directories defined in `/usr/share/daf/linker_search.txt` and `$HOME/.daf/linker_search.txt`.
  
NOTE: The linker_search.txt files are parsed like Linkfiles, so `-I` or `-L` is required before the directory path

#### Format and usage:
```
Usage: dafln <OPTION LIST>

List entries:
    -F <file>:              Load a linkfile
    <File>:                 Added as an object file from a object search directory
    -l<library>:            Adds a library from a library search directory
    -I <dir>:               Add a search directory for object files
    -L <dir>:               Add a search directory for linker libraries
    -A <dir>:               Adds a directory to the object file whitelist
    -o <file>:              Set output file name
    -X --help:              Print list of linker types
    -X <link_type> --help:  Print help for specific linker type
    -X <link_type> <exec>:  Set the linker type and what program to use
    -x <arg>                Pass a single arg to the linker
```

Note that also library files can be registered as object files.
Libraries passed as files will be searched for in the object file search directories, and will be added even in static libraries.
The filter is used to control what object files are included. With a filter, only whitelisted object files are linked.
If you wish to whitelist libraries as well, use `-A -l`

#### Linker types
By default, the linker type is *gnu_link_linux* using the gnu ld software to link.
However, you may want to change the linker type used when on other platforms, or when archiving / using certain libraries.
Linker types have their own extra parameters and format the arguments passed to the program differently.
Therefore different executables must be used for different linker types, so each type gives you a suggestion on it's help page.
Writing `-X --help` gives you a list of the linker types available. Writing `-X <link_type> --help` gives more information about a specific linker type.
The information for the default liker types are listed here:

##### gnu_link_linux
```
example program: ld
extra parameters:
    -shared
    -linkable <library>
or:
    -rpath <dir>
desc: Links an executable or a shared library with all the input object files and libraries
When linkng a shared library, the output is the soname of the library.
By supplying `-linkable`, the output becomes a symlink to the linkable file.
This is to maintain compatability with windows.
```

##### static_ar
```
example_program: ar
warnings given: For every library (-l)
desc: Packs object files into a static library
```
