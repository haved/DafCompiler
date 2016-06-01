## The Daf Compiler
A compiler for turning code in the daf langauge into c++

###### What is daf?
It's like java, only that you can put stuff on the stack. No garbage collection and compiled to machine code. 
Explicit mutability and built-in smart pointers (and dumb pointers!)  
What daf stands for, you ask? I dunno. **Definitely Adequately Functional**, perhaps?

###### Cool example?
So far it's just a preprocessor with a macro system. The macro system is cool, though! Use --preproc to only do preprocessing on an input file, and save the output as a file.  
  
**Example of macros**
```
#macro TRANSMIT<whatToDo<something>,somethingElse> ==START== #whatToDo<#somethingElse> ==END_OF_TRANSMISSION==

#macro DoSomething I'm doing #something!

printf("#TRANSMIT<#DoSomething,cheese>");
```
   
The previous code yields as a result (after preprocessing):
```
printf("==START== I'm doing cheese! ==END_OF_TRANSMISSION==");
```
  
**Example of if-flow control**
```
#macro not<value> #if #value #then 0 #else 1 #endif

//Obviously false, (sorry bash)
#macro MyValue1 0
//1 meaning true
#macro MyValue2 1 

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