
(* ==== Defables ==== *)

type infix_operator =
  | Plus | Minus | Mult | Divide | Access_Operator

type bare_defable =
  | Integer_Literal of int
  | Real_Literal of float
  | Infix_Operator of infix_operator * defable * defable
  | Identfier of string
  | Def_Literal of parameter list * return_type * defable

and defable = bare_defable * Span.span_t

(* ==== Scopes and statements ==== *)

and statement = bare_statement * Span.span_t

and bare_statement =
  | NopStatement
  | If of defable * statement * statement
  | DefinitionStatement of bare_definition
  | ExpressionStatement of defable

(* ==== Def stuff ===== *)

and return_modifier =
  | Value_Ret | Ref_Ret | Mut_Ref_Ret

and return_type = (return_modifier * defable option) option

and parameter_modifier =
  | Normal_Param | Let_Param | Mut_Param | Move_Param | Copy_Param | Uncrt_Param | Dtor_Param

and bare_parameter =
  | Value_Param of parameter_modifier * string * defable
  | Type_Inferred_Value_Param of parameter_modifier * string * string
  | Def_Param of string * parameter list * return_type

and parameter = bare_parameter * Span.span_t

(* ==== Definitions ==== *)

and bare_definition =
  | Def of string * parameter list * return_type * defable option

and definition = bool * bare_definition * Span.span_t

let string_of_infix_operator defin = match defin with
  | Plus -> "+"
  | Minus -> "-"
  | Mult -> "*"
  | Divide -> "/"
  | Access_Operator -> "."

open Printf

let string_of_bare_definition bare_defin = "bare_definition"

let string_of_definition (pub,bare_defin,span) =
  (Printf.sprintf "%s%s" (if pub then "pub " else "") (string_of_bare_definition bare_defin))
