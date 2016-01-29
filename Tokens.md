## Tokens in the daf parser
When reading the daf file, every word and special character is turned into a token.
The token stores information about its meaning, its location in the file, and its text, if it's not a keyword.

#### Compiler keyword tokens
* **#import** #import, special
* **#using** #export, special
* **##cpp** ##cpp, special
* **##header**, ##header, special
* **extern** extern
* **type** type
* **field** field

#### Declaration Tokens
* **pub** pub
* **prot** prot
* **let** let
* **mut** mut
* **def** def
* **uncertain** uncertain
* **assign** =
* **colonAssign** :=
* **colon** :
* **semicolon** ;
* **func** func
* **inline** inline
* **leftParen** \(
* **rightParen** \)
* **scopeStart** \{
* **scopeEnd** \}

#### Classes
* **class** class
* **abstract** abstract
* **extends** extends
* **implements** implements
* **interface** interface
* **method** method
* **this** this
* **const** const
* **virtual** virtual
* **override** override
* **destructor** ~

#### Control Statemets
* **if** if
* **else** else
* **elselse** elselse
* **for** for
* **while** while
* **do** do
* **continue** continue
* **break** break
* **retry** retry
* **return** return

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
* **address** &
* **shared** shared
* **new** new
* **delete** delete
* **leftBracket** \[
* **rightBracket** \]
* **bracketPair** \[\]
* **dumb**

#### Value tokes
* **identifier** *special*, any text starting in 'a-z', 'A-Z' or '_', with or without '0-9'
* **integer_literal** *special*
* **real_literal** *special*
* **string_literal** *special*
* **true** true
* **false** false
* **null** null

#### Operator tokens
* **classAccess** .
* **dereference** @
* **pointerAccess** ->
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
