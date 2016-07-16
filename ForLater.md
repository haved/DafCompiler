In this file I will note things to rememeber for later. I realized my current project had so much complexity I really could do without.
Let's just say there is a story behind every point on this list.

- Use the funky parsing stuff from the llvm tutorial. By funky I also mean simple
- Don't make primitive types mutable (why did I to begin with??)
- Don't use classes for parsing, use neatly nested recursive functions
- Space the functions out. Make 'em smaller. A good reason not to use uneducated goto (i.e. return and break)
- Decide on language features and syntax before you start
- Write stuff down
- Learn from your mistakes
- Massivly simplify lexer and syxer
- Maybe clam the preprocessor just a tad
- Don't make buffers all over the place. A current and a lookahead should be all you need.
- Don't have a null token and an advance function that returns false at EOF. Have an EOF token instead


I'll just note the plan for binary operator recursivce right hand side parsing right here.

#####Testcases:
1. a + b * c + d   ==  a + (b * c + d)
2. a * b + c       ==  (a * b) + c
3. a - c * b || g  ==  (a - (c * b)) || g

######The first case:
- Call the function: LHS = a, prevOpLevel = 0
- while(true) {
 - op = + //Precedence 30
 - **eat op token**
 - RHS = b
 - nextOp = * //Precedence 40
 - 
- }

######The bottom case:
- Call the function: LHS = a, prevOpLevel = 0
- while(true) {
 - op = - //Precedence 30
 - **eat op token**
 - RHS = c
 - nextOp = * //Precedence 40
 - Recursion: LHS = c, prevOpLevel = 30
  - op = * //Precedence 40
  - **eat op token**
  - RHS = b
  - nextOp = || //Precedence 10
  - return (LHS op(*) RHS) //Because the nextOpLevel < prevOpLevel
 - LHS = (LHS op(-) RHS)
- Second time while loop: LHS = a-(c*b)
 - op = || //Precedence 10
 - **eat op token**
 - RHS = g
 - nextOp = null //hehe
 - if(nextOp == null)
    return (LHS op RHS) //(a-(c*b)) || g
- }

Basic idea:
the function continues eating until it meets a precedence higher than the previous operator's, or a precedece lower than the prevOpLevel


