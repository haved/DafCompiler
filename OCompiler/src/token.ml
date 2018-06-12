type token =
  | Pub | Let | Def | With | As | Mut | Uncrt | Move | Copy
  | Class | Trait | Enum | Prot
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

  | Declare | Module_Access
  | Lsl | Ars | Lsr | Logical_And | Logical_Or
  | Equals | Not_Equals | Greater_Or_Equal | Lower_Or_Equal | Plus_Plus | Minus_Minus

  | Mut_Ref

  | Identifier of string | String_Literal of string | Integer_Literal of int | Real_Literal of float
  | End_Token | Error_Token of char
