
type token =
  | Pub | Let | Def | With | As | Mut | Uncrt | Move | Copy
  | Class | Trait | Namespace | Enum | Prot
  | Ctor | Dtor | This | This_Type
  | Virt | Override

  | If | Else | For | While | Do | Match
  | Continue | Break | Retry | Return

  | Char
  | I8 | U8 | I16 | U16 | I32 | U32 | I64 | U64
  | Usize | Isize | Bool | F32 | F64

  | Sizeof | Typeof | Lengthof
  | True | False | Null

  | Assign | Type_Separator | Statement_End | Left_Paren | Comma | Right_Paren
  | Scope_Start | Scope_End | Class_Access | Dereference
  | Left_Bracket | Right_Bracket | Type_Infered

  | Plus | Minus | Mult | Divide | Modulo
  | Ref | Bitwise_Or | Bitwise_Xor | Not | Bitwise_Not
  | Lower | Greater | Q_mark

  | Declare
  | Lsl | Asr | Lsr | Logical_And | Logical_Or
  | Equals | Not_Equals | Greater_Or_Equal | Lower_Or_Equal | Plus_Plus | Minus_Minus

  | Mut_Ref

  | Identifier of string | String_Literal of string | Integer_Literal of int | Real_Literal of float
  | Error of char


let token_to_string token =
  match token with
  | Pub -> "pub" | Let -> "let" | Def -> "def" | With -> "with" | As -> "as" | Mut -> "mut" | Uncrt -> "uncrt" | Move -> "move" | Copy -> "copy"
  | Class -> "class" | Trait -> "trait" | Namespace -> "namespace" | Enum -> "enum" | Prot -> "prot"
  | Ctor -> "ctor" | Dtor -> "dtor" | This -> "this" | This_Type -> "This"
  | Virt -> "virt" | Override -> "override"

  | If -> "if" | Else -> "else" | For -> "for" | While -> "while" | Do -> "do" | Match -> "match"
  | Continue -> "continue" | Break -> "break" | Retry -> "retry" | Return -> "return"

  | Char -> "char"
  | I8 -> "i8" | U8 -> "u8" | I16 -> "i16" | U16 -> "u16" | I32 -> "i32" | U32 -> "u32" | I64 -> "i64" | U64 -> "u64"
  | Usize -> "usize" | Isize -> "isize" | Bool -> "bool" | F32 -> "f32" | F64 -> "f64"

  | Sizeof -> "sizeof" | Typeof -> "typeof" | Lengthof -> "lengthof"
  | True -> "true" | False -> "false" | Null -> "null"

  | Assign -> "=" | Type_Separator -> ":" | Statement_End -> ";" | Left_Paren -> "(" | Comma -> "," | Right_Paren -> ")"
  | Scope_Start -> "{" | Scope_End -> "}" | Class_Access -> "." | Dereference -> "@"
  | Left_Bracket -> "[" | Right_Bracket -> "]" | Type_Infered -> "$"

  | Plus -> "+" | Minus -> "-" | Mult -> "*" | Divide -> "/" | Modulo -> "%"
  | Ref -> "&" | Bitwise_Or -> "|" | Bitwise_Xor -> "^" | Not -> "!" | Bitwise_Not -> "~"
  | Lower -> "<" | Greater -> ">" | Q_mark -> "?"

  | Declare -> ":="
  | Lsl -> "<<" | Asr -> ">>" | Lsr -> ">>" | Logical_And -> "&&" | Logical_Or -> "||"
  | Equals -> "=" | Not_Equals -> "!=" | Greater_Or_Equal -> ">=" | Lower_Or_Equal -> "<=" | Plus_Plus -> "++" | Minus_Minus -> "--"

  | Mut_Ref -> "&mut"

  | Identifier string -> string
  | String_Literal string -> String.concat string ["\""; "\""]
  | Integer_Literal int -> string_of_int int
  | Real_Literal float -> string_of_float float
  | Error char -> "ERROR_TOKEN"
