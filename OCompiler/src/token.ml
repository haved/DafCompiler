
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

  | Assign | Type_Separator | Declare | Statement_End | Left_Paren | Comma | Right_Paren
  | Scope_Start | Scope_End | Left_Bracket | Right_Bracket | Type_Infered

  | Plus | Minus | Mult | Divide | Modulo
  | Lsl | Asr | Lsr
  | Ref | Mut_Ref | Dereference | Class_Access
  | Bitwise_Or | Bitwise_Xor | Not | Bitwise_Not
  | Logical_And | Logical_Or
  | Equals | Not_Equals | Greater_Or_Equal | Lower_Or_Equal | Lower | Greater
  | Q_mark

  | Plus_Plus | Minus_Minus

  | Identifier of string | String_Literal of string | Integer_Literal of int | Real_Literal of float
  | Error of char


let string_to_token text =
  match text with
  | "pub" -> Pub | "let" -> Let | "def" -> Def | "with" -> With | "as" -> As | "mut" -> Mut | "uncrt" -> Uncrt | "move" -> Move | "copy" -> Copy
  | "class" -> Class | "trait" -> Trait | "namespace" -> Namespace | "enum" -> Enum | "prot" -> Prot
  | "ctor" -> Ctor | "dtor" -> Dtor | "this" -> This | "This" -> This_Type
  | "virt" -> Virt | "override" -> Override

  | "if" -> If | "else" -> Else | "for" -> For | "while" -> While | "do" -> Do | "match" -> Match
  | "continue" -> Continue | "break" -> Break | "retry" -> Retry | "return" -> Return

  | "char" -> Char
  | "i8" -> I8 | "u8" -> U8 | "i16" -> I16 | "u16" -> U16 | "i32" -> I32 | "u32" -> U32 | "i64" -> I64 | "u64" -> U64
  | "usize" -> Usize | "isize" -> Isize | "bool" -> Bool | "f32" -> F32 | "f64" -> F64

  | "sizeof" -> Sizeof | "typeof" -> Typeof | "lengthof" -> Lengthof
  | "true" -> True | "false" -> False | "null" -> Null
  | id -> Identifier id

let char_to_token c =
  match c with
  | '=' -> Assign | ':' -> Type_Separator | ';' -> Statement_End | '(' -> Left_Paren | ',' -> Comma | ')' -> Right_Paren
  | '{' -> Scope_Start | '}' -> Scope_End | '[' -> Left_Bracket | ']' -> Right_Bracket | '$' -> Type_Infered

  | '+' -> Plus | '-' -> Minus | '*' -> Mult | '/' -> Divide | '%' -> Modulo
  | '&' -> Ref | '.' -> Class_Access | '@' -> Dereference
  | '|' -> Bitwise_Or | '^' -> Bitwise_Xor | '!' -> Not | '~' -> Bitwise_Not
  | '<' -> Lower | '>' -> Greater
  | '?' -> Q_mark
  | _ -> Error c

let rec merge_tokens_in_stream = parser
                               | [< 'Type_Separator; next_parser=merge_type_sep_in_stream >] -> next_parser
                               | [< 'Ref; next_parser=merge_ref_in_stream >] -> next_parser
                               | [< 'Assign; next_parser=merge_assign_in_stream >] -> next_parser
                               | [< 'Not; next_parser=merge_not_in_stream >] -> next_parser
                               | [< 'token; next_parser=merge_tokens_in_stream >] -> [< 'token; next_parser >]
                               | [< >] -> [< >]

and merge_type_sep_in_stream = parser
                             | [< 'Assign; next_parser=merge_tokens_in_stream >] -> [< 'Declare; next_parser >]
                             | [< next_parser=merge_tokens_in_stream >] -> [< 'Type_Separator; next_parser >]

and merge_ref_in_stream = parser
                        | [< 'Mut; next_parser=merge_tokens_in_stream >] -> [< 'Mut_Ref; next_parser >]
                        | [< next_parser=merge_tokens_in_stream >] -> [< 'Ref; next_parser >]

and merge_assign_in_stream = parser
                           | [< 'Assign; next_parser=merge_tokens_in_stream >] -> [< 'Equals; next_parser >]
                           | [< next_parser=merge_tokens_in_stream >] -> [< 'Assign; next_parser >]

and merge_not_in_stream = parser
                        | [< 'Assign; next_parser=merge_tokens_in_stream >] -> [< 'Not_Equals; next_parser >]
                        | [< next_parser=merge_tokens_in_stream >] -> [< 'Not; next_parser >]

(* TODO: lsl, asr, lsr, &&, ||, >=, <=, ++, --*)
(* TODO: Make a prettier token merging system. This parser stuff is too big*)

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

  | Assign -> "=" | Type_Separator -> ":" | Declare -> ":=" | Statement_End -> ";" | Left_Paren -> "(" | Comma -> "," | Right_Paren -> ")"
  | Scope_Start -> "{" | Scope_End -> "}" | Left_Bracket -> "[" | Right_Bracket -> "]" | Type_Infered -> "$"

  | Plus -> "+" | Minus -> "-" | Mult -> "*" | Divide -> "/" | Modulo -> "%"
  | Lsl -> "<<" | Asr -> ">>" | Lsr -> ">>"
  | Ref -> "&" | Mut_Ref -> "&mut" | Class_Access -> "." | Dereference -> "@"
  | Bitwise_Or -> "|" | Bitwise_Xor -> "^" | Not -> "!" | Bitwise_Not -> "~"
  | Logical_And -> "&&" | Logical_Or -> "||"
  | Equals -> "==" | Not_Equals -> "!=" | Greater_Or_Equal -> ">=" | Lower_Or_Equal -> "<=" | Lower -> "<" | Greater -> ">"
  | Q_mark -> "?"

  | Plus_Plus -> "++" | Minus_Minus -> "--"

  | Identifier string -> String.concat string ["_";"_"]
  | String_Literal string -> String.concat string ["\""; "\""]
  | Integer_Literal int -> string_of_int int
  | Real_Literal float -> string_of_float float
  | Error char -> Printf.sprintf "ERROR_TOKEN %c" char
