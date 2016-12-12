# Type Architecture

This is a document detailing the internal representation of types.
At the heart of the system is the Type class and the Type Parser.

When expecting a type from the Lexer, the type parser is invoked.
It looks at the tokens, and returns an optionally unique pointer to a Type.
It cleans up after itself only if wanted by the parser.
The optionally unique pointer also contains a text range to show where the type is referenced

Object types the parser can return pointers to:
* Primitive types (global types, aka not heap allocated)
* An identifier (type def, not global)
* A pointer or array type, (not global, as the memory footprint can be quite small)
* A class or trait, also being heap allocated. Knows own location, as well as its reference
