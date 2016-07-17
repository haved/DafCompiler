In this file I will note things to rememeber for later. I realized my current project had so much complexity I really could do without.
Let's just say there is a story behind every point on this list.

#####Compiler help:
- Use the funky parsing stuff from the llvm tutorial. By funky I also mean simple
- Don't make primitive types mutable (why did I to begin with??)
- Don't use classes for parsing, use neatly nested recursive functions
- Space the functions out. Make 'em smaller. A good reason not to use uneducated goto (i.e. return and break)
- Decide on language features and syntax before you start
- Write stuff down
- Learn from your mistakes
- Massivly simplify lexer and syxer
- Maybe calm the preprocessor just a tad
- Don't make buffers all over the place. A current and a lookahead should be all you need.
- Don't have a null token and an advance function that returns false at EOF. Have an EOF token instead

######Main problems:
- **Find a way of storing libraries** (Some kind of header that is still a .daf file?)
- **Find a way of making generics work**

#####Langauge ideas and quirks:
- Maybe allow for parameters that are just (a:&, b:&mut) when you don't care about the pointer type?
 - Maybe even allow for returning these, that may be impicitly cast to anything
- Maybe cast like (exp **as** type)
- Use a module system based on the new import system
 - Have a **module** keyword instead of namespaces
- Use (int, &&char):int to show return type
- Remember that && means pointer to pointer, even though the token is bitwise and
- Add **typedef** as well
- Add **&move** as a refrence type to procedures
 - Automaticly use it when the param is an rvalue
- Add **&shared** and **&unique** pointers
- When passing a refrence you don't have the **&** symbol in front. (That would be a pointer)
 - This means mut and move refs have the words written before the expression
- Allow for statements returning expressions?
- Demand that **virtual** classes have **virtual** destructors
- Have a **base** keyword
- Have a **final** keyword
- Remember **const** keyword for methods
- Objects themselves can't be immutable (unless const destructor), and are implicitly let mutable
 - This also means the deletion of objects on the heap must happen to a pointer with mutability (unless const destructor)
  - classes with const destructors can only contain primitive fields (that includes normal pointers)
   - Also superclasses have to have const virtual destructors. 
- Require the destructor to be virtual to allow polymorphic pointers
 - As long as a superclass has a virtual destructor, the destructor of all it's subclasses will count as virtual
- Require semicolon after every definition (let, def, typedef, import, class, module) (not methods, but def's inside classes? :/)
- Parameters in signatures don't need a name. i.e. (&int):int is a legal signature. (But it's still legal)
 - But if the parameter is a refrence: (&move:&int):int is the syntax
- Add a way of choosing constructor (other than parameter type)

#####Import syntax suggestion:
import daf.memory; //string.strlen()
import * from daf.memory; //strlen();
import string from daf.memory; //strlen();
import string as str from daf.memory; //str.strlen();
import string as is from daf.memory; //string.strlen();
import string as is, allocate from daf.memory;
import string, allocate as are from daf.memory;
import * as are from daf.memory; //Same as import daf.memory
If something in a file is outside of a module, it'll allways be imported, without a name
