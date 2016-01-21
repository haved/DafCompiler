## Parsing Nodes
This is a list of all nodes that the daf parser will parse a daf into. The node structure is used when importing .daf files or creating .h and .cpp files.

#### Main nodes
* **(rootNode, ...)**	The root node. Contains all the file
* **(import, fileName:String, file:ParsedFile)**	Temporary
* **(using, fileName:String, file:ParsedFile)**	Just like import
* **(cpp, text:String)** Inline cpp code for the .cpp file
* **(header, text:String)** Inline cpp code for the .h file

#### Main nodes
* **(pub, whatever)**
* **(prot, whatever)**
* **(type, name:String)**
* **(extern func, funcName:String, returnType:type)**

#### Scope nodes
* **(scope, ...)**

#### Variable nodes
* **(let, mut|non-mut, fieldType:type, value:String)**
* **(def, defType:type, value:String)**

#### Class nodes
* **(class, name:String, fieldList, methodList)**
* **
