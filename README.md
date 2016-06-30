## The Daf Compiler
A compiler for turning code in the daf langauge into c++

###### What is daf?
It's like java, only that you can put stuff on the stack. No garbage collection and compiled to machine code. 
Explicit mutability and built-in smart pointers (and dumb pointers!)  
What daf stands for, you ask? I dunno. **Definitely Adequately Functional**, perhaps?

###### Cool example?
So far it's just a preprocessor with a macro system. The macro system is cool, though! Use --preproc to only do preprocessing on an input file, and save the output as a file.  

**Grand preprocessor FizzBuzz**
```
#macro Next<val, max>$#(#val 3 % 0 == "Fizz" "" ?)#(#val 5 % 0 == "Buzz" ""?)#(#val 3 % #val 5 % * 0 != #val "" ?)
#if#(#val #max<><> <)#then#Next<#(#val 1 +),#max>#endif$
#Next<1, 100>
```
The code above yeilds all the numbers from 1 to 100, with every multiple of 3 replaced with 'Fizz', every multiple of 5 replaced with 'Buzz' and every multiple of both replaced with 'FizzBuzz'. A true classic! 

**Example of macros**
```
#macro TRANSMIT<whatToDo<something>,somethingElse> ==START== #whatToDo<#somethingElse> ==END_OF_TRANSMISSION==

#macro DoSomething I'm doing #something!

printf("#TRANSMIT<##DoSomething,cheese>");
```
   
The previous code yields as a result (after preprocessing):
```
printf("==START== I'm doing cheese! ==END_OF_TRANSMISSION==");
```
  
**Example of if-flow control**
```
#macro not<value> #if #value #then 0 #else 1 #endif

#macro MyValue1 0 //Obviously false, (sorry bash)
#macro MyValue2 1 //1 meaning true

#if #not<#MyValue1> #then
printf("MyValue1 is false!");
#if #MyValue2 #then
printf("But MyValue2 is true!");
#else
printf("...And so is MyValue2");
#endif
#endif

#macro ToTrueOrFalse<value> #if #value #then<>true#else<>false#endif

printf("To summarize: MyValue1 is #ToTrueOrFalse<#MyValue1> while MyValue2 is #ToTrueOrFalse<#MyValue2>");
```

The previous code yields as a result, with empty lines removed:
```
printf("MyValue1 is false!");
printf("But MyValue2 is true!");
printf("To summarize: MyValue1 is false while MyValue2 is true");

```