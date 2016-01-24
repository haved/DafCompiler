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
* **pub**
* **prot**
* **let**
* **def**
* **class**
* **abstract**
* **extends**
* **implements**
* **interface**
* **method**

#### Value tokes
* **identifier**
* **numeric value**
* **string value**
* ****

#### Operator tokens
* **plus**
* **minus**
* **mult**
* **divide**
* **modulo**
* **qMark**
* **optionSep**


