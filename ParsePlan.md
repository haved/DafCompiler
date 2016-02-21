#A plan for how parsing will be implemented
This is a plan. Hopefully it will work.  
  
###Parsed file
* **has a list of own definitions.** Normally just pub, empty definitions
* **has a list of know of classes** From a UsedFile for every #using, including oneself

###The main file
* **Extends Parsed File**
 * **definitions** Only that the main file parses all definitions, with contents
 * **list of known of classes**
* **has a list of imported definitions**
* **has a list of used definitions** Only usable from inside a scope
* **has a list of used un-imported classes** Meaning you must forward declare it
* Only one instance per file


###UsedFile
* **has a list of known of classes in a file, plus files imported by it**
* Only one instance per file
 * New instance has empty list, and **parsingProgress** = **UNPARSED**  
##### Parse()
* Check if parsing already (**parsingProgress** == **PARSING**)
 * A recursive #import has occured! Error and return!
* Return if already parsed (**parsingProgress** == **PARSED**)
* Set **parsingProgress** to **PARSING**;
* The tokens are gotten for the file
* Goes through the tokens looking for #import
 * When found, make a UsedFile for that as well
 * Add it's contents to yourself
* Set **parsingProgress** to **PARSED**;
