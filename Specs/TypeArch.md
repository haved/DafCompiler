## Type architecture

Actually, forget about that bellow. Here is the real idea.
Whenever you parse a type, the result is an object with a TextRange, plus a string or Type pointer. The method isDefined() tells you weather or not you have to find the typedef referenced by the string
The type parser will upon finding more advanced Type expressions, (classes, traits, arrays, pointers etc.), heap allocate a global instance pointed to by the TextRange.
The type parser will do its best to re-use the global, immutable Type instances, so the list is split into categories to make iteration easier.
