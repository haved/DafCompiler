## All keywords in daf
This is a list of all the keywords you find in daf. The file ParsingNodes.md shows how the keywords are parsed into a lisp-style data structure.  
  
#### Compiler messages
* **#import** "File.daf"	Includes File.h in the header.
* **#using** "File.daf"		Includes File.h in the implementation file.
* **#cpp**		 						Starts cpp mode that will be put directly into the source file.
* **#header**	Starts cpp mode that instead will be put in the header.
* **#end**								Ends cpp mode for both #cpp and #header.

#### Extern
* **extern func** *MyFunc* Makes use of the c++ function "MyFunc" possible.
* **extern type** *MyType*	Makes use of the c++ type "MyType" possible.
* **extern field** *MyValue*	Makes use of the c++ field/define "MyValue" possible.
* **extern class** *MyClass*	Makes use of the c++ class "MyClass" and *any* method possible.

#### Declarations
* **pub**	Makes something public, either in a class or a header.
* **prot**	Makes something in a class protected.
* **let**	Sets a value. Either a static one or in scope.
* **def**	Defines a value. Compile time.
* **uncertain**	A value can be declared without being set.

#### Classes
* **class** *MyClass*	Makes a new class "MyClass".
* **abstract**	Makes a class abstract.
* **extends**	Makes the class extend from another
* **interface** *MyInterface*	Makes an interface "MyInterface".
* **implements**	Makes a class implement an interface

#### Functions
* **func** *FunctionName(arg1:type1)*	Makes a function
* **method** *MethodName()*	Makes a method (in classes only)
* **this**	Pointer to class method belongs to. Passed implicitly.
* **const**	Lets a method take an immutable pointer to **this**
* **inline** Marks a function to be inlined

#### Control Statements
* **if**
* **else**
* **while**
* **do**
* **for**
* **break**
* **continue**
* **retry**	Just like **continue**, only the iterator isn't iterated
* **elselse**	Called if the else wasn't called.

#### Values
* **true**
* **false**
* **null**	Null pointer

#### Types
All types should be obvoius as to sign and bit count. Think java.

* **uint8** - **uint64**
* **int8**  - **int64**
* **char**
* **ubyte**
* **sbyte**
* **ushort**
* **short**
* **uint**
* **int**
* **ulong**
* **long**
* **usize**
* **boolean**

#### Special character words
* **=** assignment
* **:=** autoDecl assignment
* **.** accessing class fields
* **^** dereference
* **->** accessing fields in pointer
* **;** semicolon between statements
* **:** colon between identifier and type, as well as in **? :**
* **?** quesiton sytnax like in java
* **(** function paramerters start, left parameter
* **)** function paramerters end, right parameter
* **[** left bracket
* **]** right bracket
* **+** plus
* **-** minus
* **/** division
* **\*** multiplication
* **%** modulo
* **<** less than as well as template types
* **>** greater than as well as template types
* **<=** less than or equals
* **>=** greater than or equals
* **==** equals
* **!=** not equals
* **!** not

#### Pointers
* **new**
* **delete**
* **delete[]**
* **&**

Putting '&' in front of the type makes a const pointer.
Putting '&mut' makes a mutable pointer.
Putting these in front of a paramter name passes the parameter by reference.
Examples:
```
let mut i:=0; //Type int32
let mut iPtr:=&mut i; //Get the address of i
let mut intPointer:&mut int32; //Define a mutable int32 pointer
intPointer=&mut i; //or just iPtr for that matter

//times is a mutable int pointer
//name is a const referance to a string
//ref is a mutable reference to a mutable int pointer. (Why even do that??)
func MyFunc(times:&mut int, &name:String, &mut ref:&mut int) {}
```

