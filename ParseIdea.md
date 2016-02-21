##How parsing might be done
Written down do avoid later realizations of self incompetance
* **The main file has a list of own definitions and their contents**
* **It also has a list of imported definitions**
* **As well as a list of used definitions**
* **It also has a list of classes known about through #using**
* The main file is turned into tokens.
* A main file automaticly uses itself. The source file uses the header after all.
 * When maing a **UsedFile** for the main file, both public and private classes are added to classes you know about
* You start going through, Adding definitions along the way
* When you meet an #import:
 * Parse the file as **IncludedFile**
 * Add it's list of defined classes from #using statements to yours, if it's not already there
 * Add all the pub definitions of the **IncludedFile** to your imported defintions
* When you meet an #using:
 * Parse the file as **IncludedFile**
 * Add all the pub definitions of the **IncludedFile** to your used definitions
 * Add the **IncludedFile**s list of defined classes from #using statements to yours, if it's not already there
 * Also parse the file as a UsedFile, and add it's 
* The imports and uses are turned into definitions as well, to be added as #include
* Inline code is never imported. It stays in it's file
  
#####How does an IncludedFile work?
* **Only one instance per file**
* **The main file is one of them, maybe? Inheritance?**
* **They have a list of pub definitions.**
* **It also has a list of public classes known about through #using**
* When you parse a file it is first tokenized
* It goes through and adds empty public definitions
* If it meets #import, parse it as includedFile
 * If it's already parsing: Recursive importing is illegal
 * Add it's pub definitions to you
* If it meets #using
 * Parse the file as a UsedFile
  * Add its classes to your own list of classes known about

#####How does a UsedFile work?
* **Only one instance per file**
* **List of pub classes defined in the file**
* If it's already parsed, no problem
* If it's already parsing, no problem either
* Go through the file and find all public classes defined.
 * If an **IncludedFile** already exist for it and is done parsing, use its pub defintions list.
* If you find a #import, go into it as a UsedFile as well, and add it's contents to your own list


#### When adding definitions
Note to self: Should probably have an interface called "Type"
* If you are using a type, go through the list of imported definitions and/or definitions to find it. (Type of pointers too)
* If that failed and it's a pointer to a class, go through the list of classes known about through #using
 * If this is the main file, add it to the list of used as pointer
  * This will be used to make class pre-definitions in the header
* Inside a scope, everyting is fair game. As long as it's defined somewhere
