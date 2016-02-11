## All keywords in daf
This is a list of all the keywords you find in daf. The file ParsingNodes.md shows how the keywords are parsed into a lisp-style data structure.  
  
#### Compiler messages
* **#import** "File.daf"	Includes File.h in the header.
* **#using** "File.daf"		Includes File.h in the implementation file.
* **##cpp**		 						Starts cpp mode that will be put directly into the source file.
* **##header**	Starts cpp mode that instead will be put in the header.
* **##end**								Ends cpp mode for both #cpp and #header.

#### Extern
* **extern func** *MyFunc* Makes use of the c++ function "MyFunc" possible.
* **extern type** *MyType*	Makes use of the c++ type "MyType" possible.
* **extern field** *MyValue*	Makes use of the c++ field/define "MyValue" possible.
* **extern class** *MyClass*	Makes use of the c++ class "MyClass" and *any* method possible.

#### Declarations
* **pub**	Makes something public, either in a class or a header.
* **prot**	Makes something in a class protected.
* **let**	Sets a value. Either a static one or in scope.
* **mut**	Marks a veriable as mutable.
* **def**	Defines a value. Compile time.
* **uncertain**	A value can be declared without being set.
* **=** 	Assignment.
* **:=**	Type deducting asignment.
* **:**	Colon, between identifier and type, also in ?: statements.
* **;** Semicolon after statements and declarations.
* **func** A function definition.
* **\(**
* **,**
* **\)**
* **\{** Start scope.
* **\}** End scope.

#### Classes
* **class** *MyClass*	Makes a new class "MyClass".
* **abstract**	Makes a class abstract.
* **extends**	Makes the class extend from another
* **interface** *MyInterface*	Makes an interface "MyInterface".
* **implements**	Makes a class implement an interface
* **method** In classes only
* **this**	Pointer to class.
* **const**	**this** is not mutable.
* **virtual** Makes a method overrideable.
* **override** Marks an override as such.
* **~** A Destructor.

#### Control Statements
* **if**
* **else**
* **elselse**	Called if the else wasn't called.
* **for**
* **while**
* **do**
* **continue**
* **break**
* **retry**	Just like **continue**, only the iterator isn't iterated
* **return**

#### Types
* **char** char
* **ubyte** byte
* **short** short
* **ushort** ushort
* **int** int
* **uint** uint
* **long** long
* **ulong** ulong
* **int8** int8
* **uint8** uint8
* **int16** int16
* **uint16** uint16
* **int32** int32
* **uint32** uint32
* **int64** int64
* **uint64** uint64
* **usize** usize
* **boolean** boolean
* **float** float
* **double** double

#### Pointers
* **&**
* **shared** For shared pointers. Always **mut**
* **new**
* **delete**
* **delete[]**
* **[ n ]** used to access the 'n'th element of an array
* **dumb** A built in func for making a dumb &shared

#### Values
* **true**
* **false**
* **null**	Null pointer.

#### Operators
* **.** access public fields and methods from classes.
* **@** dereference a pointer.
* **->** access public fields and methods from pointers.
* **+** plus
* **-** minus
* **\*** multiplication
* **/** division
* **%** modulo
* **<<** left shift
* **>>** arithmetic shift right
* **>>>** logical shift right
* **&nd** bitwise and
* **&&** logical and
* **|** bitwise or
* **||** logical or
* **^** xor
* **!** not
* **+=** plus equals
* **-=** minus equals
* **\*=** multiplication equals
* **/=** division equals
* **%=** modulo equals
* **<<=** left shift equals
* **>>=** arithmetic shift right equals
* **>>>=** logical shift right equals
* **&nd=** bitwise and equals
* **&&=** logical and equals
* **|=** bitwise or equals
* **||=** logical or equals
* **^=** xor equals
* **!=** not equals
* **==** equals
* **<** lower
* **<=** lower than or equals
* **>** greater
* **>=** greater than or equals
* **?** question mark for ?: syntax


#### Using pointers:
Putting '&' in front of the type makes a const pointer.
Putting '&mut' makes a mutable pointer.
Putting these in front of a paramter name passes the parameter by reference.
Putting '&shared' makes a shared mutable pointer.
Examples:
```
let mut i:=0; //Type int32
let mut iPtr:=&mut i; //Get the address of i
let mut intPointer:&mut int32; //Define a mutable int32 pointer
intPointer=&mut i; //or just iPtr for that matter
let myShared:&shared int32=intPointer; //Auto conversion from mutable pointer to shared pointer
let dumbPointer:&shared int32=dumb(myShared); //When deleted, nothing happens.

//times is a mutable int pointer
//name is a const referance to a string
//ref is a mutable reference to a mutable int pointer. (Why even do that??)
func MyFunc(times:&mut int, &name:String, &mut ref:&mut int) {}
```

