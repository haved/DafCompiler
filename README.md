## The daf compiler
A systems programming language for me to sink too much time into.

### Project structure
 - **Ideas/** - Language design is hard, ok?
 - **Tools/** - Extra files for tooling
 - **Old/** - Here there be dragons

### Progress
Check out my Trello board [here](https://trello.com/b/bXCZLvBz "Daf trello board").

### Building
#### Dependencies

 - Install [opam](https://opam.ocaml.org/ "Opam website")
 - `$ opam init && opam install oasis`
 
#### Compiling

In the `/Compiler` folder:
 ```
 oasis setup -setup-update dynamic
 make
 ```
 
You should now have a symlink called `dafc.native`, which is a self contained binary

#### Cleaning up
After running oasis and make, you will have the following files in `./Compiler`:
```
 - configure
 - Makefile
 - setup.ml
 - setup.data
 - setup.log
 - _build/
 - dafc.native
```
Remove them, and everything should be clean
Remeber that `dafc.native` is a symlink to the actual binary in `_build/`

### Testing
N/A

### Installing
N/A
