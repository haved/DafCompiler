
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
  | Scope_Start | Scope_End | Left_Bracket | Right_Bracket | Type_Infered

  | Plus | Minus | Mult | Divide | Modulo
  | Lsl | Asr | Lsr
  | Ref | Dereference | Class_Access
  | Bitwise_Or | Bitwise_Xor | Not | Bitwise_Not
  | Logical_And | Logical_Or
  | Equals | Not_Equals | Greater_Or_Equal | Lower_Or_Equal | Lower | Greater
  | Q_mark

  | Plus_Plus | Minus_Minus

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
  | Scope_Start -> "{" | Scope_End -> "}" | Left_Bracket -> "[" | Right_Bracket -> "]" | Type_Infered -> "$"

  | Plus -> "+" | Minus -> "-" | Mult -> "*" | Divide -> "/" | Modulo -> "%"
  | Lsl -> "<<" | Asr -> ">>" | Lsr -> ">>>"
  | Ref -> "&" | Class_Access -> "." | Dereference -> "@"
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


type loc_t = {line:int; col:int}
type span_t = {loc:loc_t; len:int;}
type token_with_span = (token * span_t)

let span (loc:loc_t) (len:int) :span_t = {loc=loc;len=len}

let span_loc span = span.loc
let span_end span = {line=span.loc.line; col=span.loc.col+span.len}
let span_combine head tail = {loc=head.loc; len=tail.loc.col+tail.len-head.loc.col}

let string_to_token (text : string) (loc : loc_t) : token_with_span =
  let tok = match text with
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
  | id -> Identifier id in
  (tok, {loc=loc; len=String.length text})

let char_to_token (c : char) (loc : loc_t) : token_with_span =
  let tok = match c with
  | '=' -> Assign | ':' -> Type_Separator | ';' -> Statement_End | '(' -> Left_Paren | ',' -> Comma | ')' -> Right_Paren
  | '{' -> Scope_Start | '}' -> Scope_End | '[' -> Left_Bracket | ']' -> Right_Bracket | '$' -> Type_Infered

  | '+' -> Plus | '-' -> Minus | '*' -> Mult | '/' -> Divide | '%' -> Modulo
  | '&' -> Ref | '.' -> Class_Access | '@' -> Dereference
  | '|' -> Bitwise_Or | '^' -> Bitwise_Xor | '!' -> Not | '~' -> Bitwise_Not
  | '<' -> Lower | '>' -> Greater
  | '?' -> Q_mark
  | _ -> Error c in
  (tok, {loc=loc; len=1})

type merge_t = {input:token*token; output:token}
let merges = [
  {input=(Assign,     Assign);     output=Equals};
  {input=(Not,        Assign);     output=Not_Equals};
  {input=(Lower,      Lower);      output=Lsl};
  {input=(Greater,    Greater);    output=Asr};
  {input=(Asr,        Greater);    output=Lsr};
  {input=(Ref,        Ref);        output=Logical_And};
  {input=(Bitwise_Or, Bitwise_Or); output=Logical_Or};
  {input=(Lower,      Assign);     output=Lower_Or_Equal};
  {input=(Greater,    Assign);     output=Greater_Or_Equal};
  {input=(Plus,  Plus);  output=Plus_Plus};
  {input=(Minus, Minus); output=Minus_Minus};]

let get_merge (tok1, tok1_span) (tok2, tok2_span) =
  if (span_end tok1_span) <> (span_loc tok2_span) then None else
    let rec check_list list =
      match list with
      | {input=(match1, match2); output=out} :: rest ->
        if match1 == tok1 && match2 == tok2 then Some (out, span_combine tok1_span tok2_span) else check_list rest
      | [] -> None
    in
    check_list merges

let merge_tokens_in_stream stream =
  let rec maybe_merge_with_one tok1:token_with_span =
    match Stream.peek stream with
    | Some tok2 -> (
      match get_merge tok1 tok2 with
        | Some merge ->
          Stream.junk stream; (* Eat tok2 *)
          maybe_merge_with_one merge
        | None -> tok1 (* tok2 is still in stream *)
    )
    | None -> tok1 in
  let next _ =
    match Stream.peek stream with
    | Some tok1 ->
      Stream.junk stream;
      Some (maybe_merge_with_one tok1)
    | None -> None
  in Stream.from (next)
