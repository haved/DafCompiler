#A plan for how parsing will be implemented
This is a plan. Hopefully it will work.  
The file that is turned into c++ source and header is used in a new instance of a main file.
The main file is gotten from the Input file helper, making sure only one Input file 
(or subclass) exists per file.
From there, the main files parse method is called, implemented in the InputFile.
All imports and uses are handled from there.
Finally the main file has a method for turning the parsed info into c++ source and header files.
  
###Input file
* **has a list of own definitions.** Normally just public, empty definitions
* **has a list of know of classes** From a UsedFile for every #using, including oneself
* **parsingProgress:enum**
* Only one instance per file
 * New instance has empty lists and **parsingProgress** = **UNPARSED**  

##### Parse()
* Check if parsing already (**parsingProgress** == **PARSING**)
 * A recursive #import has occured! Error and return!
* Return if already parsed (**parsingProgress** == **PARSED**)
* Set **parsingProgress** to **PARSING**;
* The tokens are gotten for the file
* Make a UsedFile for oneself
 * Add it's info to the list of known classes
* **GoThroughTokens();**
* Set **parsingProgress** to **PARSED**

##### virtual method GoThroughTokens()
* First add yourself as #using
* If you find #import
 * Get the Input file for it
 * Add its pub definitions to your pub definitions
* When #using is found
 * Get the UsedFile for it and parse it; publicOnly
  * Add the classes it knows about to your own list
* Go through tokens and find public defintions
 * Add them to your list without filling them with scopes
 * Make sure the types used are imported, or pointers to known of classes
  * This means a private class in this file isn't known about

###Main Input file
* **Extends Input File**
 * **definitions** Only that the main file parses all definitions, with contents
 * **list of known of classes**
* **has a list of imported definitions**
* **has a list of used definitions** Only usable from inside a scope
* **has a list of used un-imported classes** Meaning you must forward declare it
* Only one instance per file
 * New instance has empty lists as well as inherited constructor  

##### override method GoThroughTokens()
* Add yourself as #using first, getting private definitions as well
* If you find #import
 * Get the Input file for it
 * Add its public definitions to your imported definitions
* If you find #using
 * Get the Input file for it
 * Add its public definitions to your used definitions (private too if yourself)
 * Get the UsedFile of it and parse it (publicOnly unless it's this file)
  * Add the classes it knows about to your own list
* Add *all* the definitions found to your list of definitions
 * Make sure all types used in the actual definitions are imported or pointers to known of classes
 * In scope, the defintions may be imported, used or pointers to known abouts

###UsedFile
* **has a list of known of classes in a file, plus files imported by it**
* **parsingProgress:enum**
* **publicOnly:boolean**
* Only one instance per file
 * New instance has empty list, and **parsingProgress** = **UNPARSED**  

##### Parse(publicOnly:boolean)
* Check if parsing already (**parsingProgress** == **PARSING**)
 * A recursive #import has occured! Error and return!
* Return if already parsed and publicOnly implies this->publicOnly (**parsingProgress** == **PARSED** && **publicOnly ? true : this->publicOnly**)
 * (Basically just don't parse again unless you now need private as well)
* Set **parsingProgress** to **PARSING**;
* Make sure the list is empty
* The tokens are gotten for the file
* Go through the tokens looking for #import
 * When found, make a UsedFile for that as well
 * Add it's contents to yourself
* When a class definition is found, add it to your list of classes known about
 * When **publicOnly** is true (it'll be most of the time), only add public classes
* Set **parsingProgress** to **PARSED**;
