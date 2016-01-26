## Tokens in the daf parser
When reading the daf file, every word and special character is turned into a token.
The token stores information about its meaning, its location in the file, and its text, if it's not a keyword.

#### Compiler keyword tokens
* **#import** #import, special
* **#using** #export, special
* **#cpp** #cpp, special
* **#header**, #header, special
* **extern** extern
* **type** type
* **field** field
* **func** func
* **class** class

#### Main tokens
* **pub** pub
* **prot** prot
* **let** let
* **mut** mut
* **def** def
* **assign** =
* **colonAssign** :=
* **colon** :
* **semicolon** ;
* **uncertain** uncertain
* **func**
* **leftParen**
* **rightParen**
* **new**
* **delete**

#### Classes
* **class**
* **abstract**
* **extends**
* **implements**
* **interface**
* **method**
* **destructor** ~

#### Control Statemets
* **if**
* **else**
* **elselse**
* **for**
* **while**
* **do**
* **continue**
* **break**
* **retry**
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
* **boolean** boolean
* **float** float
* **double** double

#### Pointers
* **address** &
* **shared** shared

#### Value tokes
* **identifier**
* **integer_literal**
* **real_literal**
* **string_literal**
* **true**
* **false**

#### Operator tokens
* **plus** +
* **minus** -
* **mult** *
* **divide** /
* **modulo** %
* **qMark** ?
* **shiftLeft** <<
* **arithShiftRight** >>
* **logicShiftRight** >>>
* **bitwise and** &
* **logical and** &&
* **bitwise or** |
* **logical or** ||
* **xor** ^
* **equals** ==
* **not-equals** !=
* **lower** <
* **lowerOrEquals** <=
* **greater** >
* **greaterOrEquals** >=
* **not** !
