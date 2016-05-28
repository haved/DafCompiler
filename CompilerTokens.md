##Daf preprocessor directives

This is a list of all the preprocessor directives in daf, and what they do.
*NOTE:* If you want normal text right after a directive, put <> inbetween. 

* **#macro <Macro Definition>**  -  Defines a macro until the newline
* **#macro $<Macro Definit.>$**  -  Defines a macro between the $ symbols
* **#<MacroName> <Parameters>**  -  Evaluates a macro
* **#if**			 -  Looks at the code between *#if* and *#then*
* **#then**			 -  Only executes the code until the *#endif*
* **#endif**  			 -  The end of an if - no matter what kind
* **#else**			 -  The else of an if - no matter what kind
* **#(**			 -  Marks the start of an expression
* **#)**			 -  Marks the end of an expression
* **#true**			 -  Means 1
* **#false**			 -  Means 0

#####Important:
A lone pound symbol has to be a part of a directive.
If you want a pound symobl in your code, write two of them.
This will be evaluated to one pound symbol once the code is pre processed.
Not that i macros, the code can go through the preprocessor many times.
You can therefore use 2^n pound symbols to determine how far down the line the
directive is evaluated.

####Expressions
Using **#(** and **#)** you can make an expression.
Everything is stack based, and the stack can hold both whole numbers and strings.
The expression evaluates to the final object on the stack once the **#)** is met.
Inside an expression you can use multiple operators.

Example:
``` #( #VERSION 4 5 + == #) ```

Strings must start with a letter and are split between spaces, unless
quotation marks are used.
  
#####Here is a list of operators that may be used to alter the stack.

* +    Add two numbers
* -    Subtract one number from another
* \*   Multiply two numbers
* /    Divide one number from another (whole)
* %    Modulo one number and another 
* ==   Equality check. evaluates to 1 if equal and 0 if unequal
* !=   Inequality check. See: ==
* >    Greater than. See: ==
* <    Less than.
* >=   Greater than equals
* <=   Less than equals
* !    Not. Turns 0 to 1, and everything else to 0
* len  Pops the prevoius string on the stack and pushes it's length
* dup  Duplicates the item on the top of the stack
* swp  Swaps the two top items
* exists Pops the string an pushes 1 if a macro with that name is present, else 0

NOTE: **==**, **!=** and **+** are the only operators that work with strings.

####Is my macro system turing complete yet?