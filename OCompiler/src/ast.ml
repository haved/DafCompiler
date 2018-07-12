
type infix_operator =
  | Plus | Minus | Mult | Divide | Access_Operator

type bare_defable =
  | Integer_Literal of int
  | Real_Literal of float
  | Infix_Operator of infix_operator * defable * defable
  | Identfier of string

and defable = bare_defable * Span.interval_t

(*TODO: ctor return is not a part of the signature, only function definition*)
and return_modifier =
  | Value_Ret | Ref_Ret | Mut_Ref_Ret

and return_type = (return_modifier * defable option) option

and parameter_modifier =
  | Normal_Param | Let_Param | Mut_Param | Move_Param | Copy_Param | Uncrt_Param | Dtor_Param

and bare_parameter =
  | Value_Param of parameter_modifier * string * defable
  | Type_Inferred_Value_Param of parameter_modifier * string * string
  | Def_Param of string * parameter list * return_type

and parameter = bare_parameter * Span.interval_t

and bare_definition = (*Without pub or interval*)
  | Def of {def_name:string; def_params:parameter list; def_ret:return_type; body:defable option;}

and definition = bool * bare_definition * Span.interval_t

let string_of_infix_operator defin = match defin with
  | Plus -> "+"
  | Minus -> "-"
  | Mult -> "*"
  | Divide -> "/"
  | Access_Operator -> "."
