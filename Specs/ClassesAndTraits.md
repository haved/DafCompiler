## Classes and traits in daf
In daf, a type may be a primtive, one of the many pointer types with or without an optional question mark, or a class or trait.
Classes and traits have a lot in common, but you can not make instances of traits. This means traits can include definitions of methods, without implementing them. With daf giving you full control over virtual methods, it can be difficult tu understand the rules traits and classes follow.
Firstly:
 * classes can be defined 'with' traits
 * traits can be defined 'with' classes
 
Traits are in many ways just abstract classes.
