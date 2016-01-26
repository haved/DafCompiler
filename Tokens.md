## Tokens in the daf parser
When reading the daf file, every word and special character is turned into a token.
The token stores information about its meaning, its location in the file, and its text, if it's not a keyword.

#### Compiler keyword tokens
* **#import**
* **#using**
* **extern type**
* **extern field**
* **extern func**
* **extern class**

#### Main tokens
* **pub** pub
* **prot**
* **let**
* **mut**
* **def**
* **assign** =
* **colonAssign** :=
* **colon**
* **semicolon**
* **func**
* **leftParen**
* **rightParen**
* **uncertain**
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
* **char**
* **ubyte**
* **short**
* **ushort**
* **int**
* **uint**
* **long**
* **ulong**
* **int8**
* **uint8**
* **int16**
* **uint16**
* **int32**
* **uint32**
* **int64**
* **uint64**
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
