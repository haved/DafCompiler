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