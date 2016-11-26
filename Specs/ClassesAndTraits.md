## Classes and traits in daf
In daf, a type may be a primtive, one of the many pointer types with or without an optional question mark, or a class or trait.
Classes and traits have a lot in common, but you can not make instances of traits. This means traits can include definitions of methods, without implementing them. With daf giving you full control over virtual methods, it can be difficult tu understand the rules traits and classes follow.
Firstly:
 * classes can be defined 'with' traits
 * traits can be defined 'with' classes
 
Traits are in many ways just abstract classes, in the sense that they can't be instantiated.
A function definition in a class is abstract if it lacks an expression, and only has a type.
Using 'def' defines compile time, while replacing it with 'virt' makes the function overrideable.

If a trait conatins `def myFunc:(&mut this, :int):`, see the next line. This will be a pointer to the implementing class.
If a trait conatins `def myFunc<this:class with this>:(&mut this, :int);`, the class implementing it must define, with either **def** or **virt**, a method with this being a pointer to that class.  
If a trait contains `def myFunc:=(&this, a:int):int this.value;`, the class implementing it already has an implementation that may not be overriten.  
If a trait contains `def myFunc<this:class with this>:=(&mut this, a:int):int this.myDefFunc(a);`, used when the function needs to access methods and fields in the class.  
If a trait contains `virt myFunc:(&mut this, :int);`, it can be called on a pointer to the trait, and will in the implementation in the class be a pointer to the class.  
If a trait contains `virt myFunc:=(&mut this, :int) age++;`, it may be overritten by a subclass, but can still be called on a pointer to this trait.
Virtually defined functions can't have compile time parameters, save for those in the typedef

