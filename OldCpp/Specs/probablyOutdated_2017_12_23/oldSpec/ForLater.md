In this file I will note things to rememeber for later. I realized my current project had so much complexity I really could do without.
Let's just say there is a story behind every point on this list.

####Compiler help:
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
- Have - as both an infix and prefix operator. a + -(b+3) is legal
 - This is instead of parsing the minus sign in front of a number as part of the literal
- After '(', if not followed by ')', parse an expression. Even if you later find out it's a parameter, it's parsed fine
- Compine & [mut|move|shared|unique] tokens into one of four tokens, depending on type
- Split += into + and = to make it easier
- Have some sort of assert that is used on user input
 - Make it recoverable (i.e. skipping until semicolon)
- Have some smarter way of giving errors upon EOF
- Don't just print the token when logging
- Maybe pack '()', '[]' and '{}' together when tokenizing, to make error recovery easier
- Allow logger to use same token as previous log message
- Format logging like: '<token>: error: Expected primary-expression before <token>'
- Exact rule: You can't have a polymorphic pointer to a superclass if it doesn't have a virtual destructor
- If an object is returned and used as a parameter, allocte the stack space once somehow
- Allow for warnings for:
 - Returning pointers
 - Doing pointer arithmetic
 - Assigning a pointer
 - Having null-pointers
- Also have an option for defaulting every uncertain value to X, making discovering of uncertain bugs easier.

####Main problems:
- **Find a way of storing libraries** (Some kind of header that is still a .daf file?)
- **Find a way of making generics work**

####Remember:
- You must explicitly keep methods virtual
- Also in interfaces, methods must be virtual

####Langauge ideas and quirks:
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
- Allow for statements returning expressions
- Have a **base** keyword
- Have a **final** keyword
- Remember **const** keyword for methods
- Objects themselves can't be immutable (unless const destructor)
 - This also means the deletion of objects on the heap must happen to a pointer with mutability (even with const destructor)
  - classes with const destructors can only contain primitive fields (that includes normal pointers)
- Require the destructor to be virtual to allow polymorphic pointers
 - As long as a superclass has a virtual destructor, the destructor of all it's subclasses will count as virtual
- Require semicolon after every definition (let, def, typedef, import, module)
- Parameters in signatures don't need a name. i.e. (&int):int is a legal signature. (But it's still legal)
 - But if the parameter is a refrence: (&move:&int):int is the syntax
- Add a way of calling methods on an rvalue while still evaluating to the rvalue
- Allow = on objects when declared
- Allow arrays to be initialized with a custom function
- Save array size with array, both on heap and stack
- Allow for dynamic amount of arguments and argument forwarding.


typedef a:= interface {
  a:int; //Requires a public integer
  b:virtual int; //Requires a public integer, that can be accessed with a polymorphic pointer

};

typedef b:=class implemenets a {
  pub a:int=20;
  pub b:=a;
  c:=virtual inline():int {
    b
  };
  d:virtual int= 
};