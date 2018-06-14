
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


module StringMap = Map.Make(String)
let rec string_pairlist_to_map m pairlist =
  match pairlist with
  | [] -> m
  | (str, tok) :: rest ->
    string_pairlist_to_map (StringMap.add str tok m) rest

let string_to_token_map = string_pairlist_to_map StringMap.empty [("pub", Pub); ("def", Def); ("mut", Mut)]

let string_to_token text =
  match StringMap.find_opt text string_to_token_map with
  | Some tok -> tok
  | None -> Identifier text

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

let try_merge_tokens tok1 tok2 =
  match (tok1, tok2) with
  | (Type_Separator, Assign) -> Some Declare
  | _ -> None

let rec merge_tokens_in_stream = parser
                               | [< 'Ref; next_parser=merge_ref_in_stream >] -> next_parser
                               | [< 'token; next_parser=merge_tokens_in_stream >] -> [< 'token; next_parser >]
                               | [< >] -> [< >]

and merge_ref_in_stream = parser
                        | [< 'Mut; next_parser=merge_tokens_in_stream >] -> [< 'Mut_Ref; next_parser >]
                        | [< next_parser=merge_tokens_in_stream >] -> [< 'Ref; next_parser >]

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

  | Identifier string -> string
  | String_Literal string -> String.concat string ["\""; "\""]
  | Integer_Literal int -> string_of_int int
  | Real_Literal float -> string_of_float float
  | Error char -> Printf.sprintf "ERROR_TOKEN %c" char
