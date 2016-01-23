## Parsing Nodes
This is a list of all nodes used when parsing the .daf files.
The node structure provides all the data needed to make both a source file and a header,
as well as checking for errors.
It is designed to be simple to read from, but to to be memory efficiant or particularly fast.
This compiler is written in Java, after all.  
The syntax of this file is as follows:  
* **node name** = **Function**|**Option2**|**Option3**:**Type**
* **Function** = name:string, list of **Param**, returns:Type
* **Param** = **Type**, reference:int, name:string

When typing the node system to a text file, a lisp like format is used.  
'''
(nodeName, value, (subNodeName, type, something, "Some text"))
'''

#### The Root Node
Each file has one root node. The root node is a list of definitions. 
These include class, function, static variable definitions, external definitions and inclusions.
* **Root Node** = list of **Definition**
* **Definition** = **Class** | **Function** | **StaticLet** | **Extern** | **Import** | **Inline**

#### Statements and Expressions
Statements are just pieces of code.
To run multiple pieces of code, you use a scope, which both is a statement and contains them.
Control statements such as "if" are also statements that contain a statement.
That is why "else {if(a){}}" is the same as "else if(a){}", only in a scope.
Scopes that only contain one statement are optimized away, and a suggestion is given to remove the scope.  
Expressions have a value. "2+4" is an example of an expression.
Most expressions are in statements, and expressions can contain statements with return values.
* **Statement** = **FunctionCall** | **Let** | ***Assignment** | **If** | **For** | **While** | **Do** | **Break** | **Continue** | **Return**
* **Expression** = 1+2
