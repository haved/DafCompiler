##Daf preprocessor directives

This is a list of all the preprocessor directives in daf, and what they do.
*NOTE:* If you want normal text right after a directive, put <> inbetween. 

* **#macro <Macro Definition>**  -  Defines a macro until the newline
* **#macro $<Macro Definit.>$**  -  Defines a macro between the $ symbols
* **#<Macro> <<Parameters>>**    -  Evaluates a macro
* **#if**			 -  Looks at the code between *#if* and *#then*
* **#then**			 -  Only executes the code until the *#endif*
* **#endif**  			 -  The end of an if - no matter what kind
* **#else**			 -  The else of an if - no matter what kind
* **#(**			 -  Marks the start of an expression

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

* **+**        Add two numbers
* **-**        Subtract one number from another
* **\***       Multiply two numbers
* **/**        Divide one number by another (whole)
* **%**        Modulo one number by another 
* **==**       Equality check. evaluates to 1 if equal and 0 if unequal
* **!=**       Inequality check. See: ==
* **>**        Greater than. See: ==
* **<**        Less than.
* **>=**       Greater than equals
* **<=**       Less than equals
* **!**        Not. Turns 0 to 1, and everything else to 0
* **len**      Pops the prevoius string on the stack and pushes it's length
* **?**	       Works like: <param0> ? <param1> : <param2> in java
* **toChar**   Turns the int on the stack to a char
* **toInt**    Turns the first char of the string on the stack to an int
* **substring**	Works like: <param0>.substring(<param1>, <param2>) in java
* **swap** 	 Swaps the two top items
* **dup**  	 Duplicates the item on the top of the stack
* **lineNum** 	 Pushes the current line number
* **colNum**	 Pushes the current column number
* **exists**  	 Pushes 1 if the popped string is a defined macro, else 0
* **macroStack** Pushes the size of the macro parameter stack
* **pop**	 Pops the top element

NOTE: **==**, **!=** and **+** are the only operators that work with both ints and strings.

####Is my macro system turing complete yet?