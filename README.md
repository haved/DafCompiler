## The Daf Compiler
A compiler for turning code in the daf langauge into c++

###### What is daf?
It's like java, only that you can put stuff on the stack. No garbage collection and compiled to machine code. 
Explicit mutability and built-in smart pointers (and dumb pointers!)  
What daf stands for, you ask? I dunno. **Definitely Adequately Functional**, perhaps?

###### Cool example?
So far it's just a preprocessor with a macro system. The macro system is cool, though! Use --preproc to only do preprocessing on an input file, and save the output as a file.  
  
**Example**
```
#macro TRANSMIT<whatToDo<something>,somethingElse> ==START== #whatToDo<#somethingElse> ==END_OF_TRANSMISSION==

#macro DoSomething I'm doing #something!

printf("#TRANSMIT<#DoSomething,cheese>");
```
   
The previous code yields as a result:
```
printf("==START== I'm doing cheese! ==END_OF_TRANSMISSION==");
``` 