# Design of the daf compiler written in cpp
main() passes its arguents to handleArgs(), which returns a CommandInput struct, detailing the input. It may also print the help page and exit.
The CommandInput is passed to handleCommandInput() which gives a vector of FileForParsing. It will complain if i.e. multiple files are to be compiled into one output file.
Each FileForParsing contains information about input and output, but the input is relative to a search dir.
If no search dir is supplied, '.' will be added.
assureInputFiles() will search though the search dirs finding the actual locations of the input files. Will complain if a file wasn't found.

