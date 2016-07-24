# Using the linker
## Compiling a normal program
```
dafln -F Linkfile -o myProgram
```
The Linkfile contains a list of object files and libraries used.
These files and libraries can be placed there automaticly by the daf compiler, or written manually.
`-o myProgram` sets the output file, in this case an executable. 

## Compiling a static library
```
dafln -static -F Linkfile -A ./ -o libMyLib.a
```
`-static` tells dafln that we are archiving a static library using *ar*.
`-A ./` tells dafln to only allow object files inside our current folder.
This is in case our project uses external libraries in the Linkfile, that we don't want to archive.

## Compiling a dynamic library
The object files passed must be position independent.
```
dafln -F Linkfile -A ./ -shared -linkable libMyLib.so.1.5 -o libMyLib.so.1.0
```
`-A ./` whitelists only local object files.
`-shared` tells the linker to compile a shared library.
`-linkable` adds a file used when linking programs, with the version number.
`-o` sets the soname and makes a symlink to the linkable file. This one is for runtime linking.

## Compiling a cpp project
When using c++, you often need to include standard c++ libraries. Using g++ to link does this for you.
```
dafln -g++ -F Linkfile -o myProgram
```
`-g++` uses g++ to link, and changes things internally, but dafln does it for you.