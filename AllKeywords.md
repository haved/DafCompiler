## All keywords in daf
This is a list of all the keywords you find in daf. The file ParsingNodes.md shows how the keywords are parsed into a lisp-style data structure.
#### Compiler messages
* #import "File.daf"     	Includes File.h in the header
* #using "File.daf"      	Includes File.h in the implementation file
* #cpp			 	Starts cpp mode that will be put directly into the source file
* #header               	Starts cpp mode that instead will be put in the header
* #end				Ends cpp mode for both #cpp and #header

#### classes, methods and functions
* class				Makes a new class
* abstract			Makes a class abstract
* extends 			Makes the class extend from another
* interface			Makes an interface
* implements			Makes a class implement an interface