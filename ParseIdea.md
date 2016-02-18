##How parsing might be done
Written down do avoid later realizations of self incompetance
* **The main file has a list of definitions and their contents**
* **It also has a list of classes known about through #using**
* The main file is turned into tokens.
* A main file automaticly uses itself. The source file uses the header after all.
* You start going through, Adding definitions along the way
* When you meet an #import:
 * Parse the file as includedFile
 * Add all the pub definitions of the includedFile to your imported defintions
* When you meet an #using:
 * Parse the file as includedFile
 * Add all the pub definitions of the includedFile to your used definitions
 * Also parse the file as a UsedFile
  * Add its classes to your own list of classes known about
* The imports and uses are turned into definitions as well, to be added as #include
* Inline code is never imported. It stays in it's file
  
#####How do includesFiles work?
* **They have a list of pub definitions.**
* **It also has a list of classes known about through #using**
* When you parse a file it is first tokenized
* It goes through and adds pub definitions just as any other file
* If it meets #import, parse it as includedFile
 * If it's already parsing: Recursive importing is illegal
 * Add it's pub definitions to you
* If it meets #using
 * Parse the file as a UsedFile
  * Add its classes to your own list of classes known about
 
#### When adding definitions
Note to self: Should probably have an interface called "Type"
* If you are using a type, go through the list of imported definitions to find it. (Type of pointers too)
* If that failed and it's a pointer to a class, go through the list of classes known about through #using
 * If this is the main file, add it to the list of used as pointer
  * This will be used to make class pre-definitions in the header
* Inside a scope, everyting is fair game. As long as it's defined somewhere
