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
 - Run `opam init`
 - Run `opam install oasis`
#### Building
 In the `/Compiler` folder:
 ```
 oasis setup -setup-update dynamic
 ```
 This creates the following files:
 - ./configure/
 - Makefile
 - setup.ml 
  
  
 Now to build:
 ```
 make
 ```
 
 You should now have a `dafc.native` self contained binary

### Testing
N/A

### Installing
N/A
