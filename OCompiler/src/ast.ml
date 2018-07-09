
type infix_operator =
  | Plus | Minus | Mult | Divide | Access_Operator

type defable_without_span =
  | Integer_Literal of int
  | Real_Literal of float
  | Infix_Operator of infix_operator * defable * defable

and defable = defable_without_span * Span.span_t

and return_modifier =
  | Value_Ret | Ref_Ret | Mut_Ref_Ret

and return_type = (return_modifier * defable option) option

and parameter_modifier =
  | Normal_Param | Let_Param | Mut_Param | Move_Param | Copy_Param | Uncrt_Param | Dtor_Param

and parameter =
  | Value_Param of parameter_modifier * string * defable
  | Type_Inferred_Value_Param of parameter_modifier * string * string
  | Def_Param of string * parameter list * return_type

and definition_wo_pub_span =
  | Def of {def_name:string; def_params:parameter list; def_ret:defable option; body:defable;}

and definition = bool * definition_wo_pub_span * Span.span_t

let string_of_infix_operator defin = match defin with
  | Plus -> "+"
  | Minus -> "-"
  | Mult -> "*"
  | Divide -> "/"
  | Access_Operator -> "."
