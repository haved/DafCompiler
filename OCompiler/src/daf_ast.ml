
type infix_operator =
  | Plus | Minus | Mult | Divide | Access_Operator

and prefix_operator =
  | Ref | MutRef | Pre_Increase | Pre_Decrease

and argument_modifier = Arg_Normal | Arg_Mut | Arg_Move | Arg_Copy

and argument = argument_modifier * defable * Span.span_t

and postfix_operator =
  | Post_Increase | Post_Decrease | Array_Access | FunctionCall of argument list

(* ==== Defables ==== *)

and bare_defable =
  | Identifier of string

  | Integer_Literal of int
  | Real_Literal of float
  | Def_Literal of parameter list * return_type * defable
  | Scope of statement list

  | Primitive_Type_Literal of primitive_type

  | Infix_Operator of infix_operator * defable * defable
  | Prefix_Operator of prefix_operator * defable
  | Postfix_Operator of postfix_operator * defable

and defable = bare_defable * Span.span_t

(* ==== Types ==== *)

and primitive_type =
  | U8 | I8 | U16 | I16 | U32 | I32 | U64 | I64 | F32 | F64 | USIZE | ISIZE | BOOL | CHAR

(* ==== Scopes and statements ==== *)

and statement = bare_statement * Span.span_t

and bare_statement =
  | NopStatement
  | If of defable * statement * statement option
  | DefinitionStatement of bare_definition
  | ExpressionStatement of defable
  | ReturnStatement of defable

(* ==== Def stuff ==== *)

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

and let_modifier = Normal_Let | Mut_Let

and bare_definition =
  | Def of string * parameter list * return_type * defable option
  | Let of let_modifier * string * defable option * defable option

and definition = bool * bare_definition * Span.span_t

let string_of_infix_operator defin = match defin with
  | Plus -> "+"
  | Minus -> "-"
  | Mult -> "*"
  | Divide -> "/"
  | Access_Operator -> "."

open Printf

let rec string_of_defable = function
  | _ -> "defable" (*TODO*)

and string_of_parameter_modifier = function
  | Normal_Param -> ""
  | Let_Param -> "let "
  | Mut_Param -> "mut "
  | Move_Param -> "move "
  | Copy_Param -> "copy "
  | Uncrt_Param -> "uncrt "
  | Dtor_Param -> "dtor "

and string_of_param (bare_param, _) = match bare_param with
  | Value_Param (param_modif, name, typ) ->
    Printf.sprintf "%s%s:%s" (string_of_parameter_modifier param_modif) name (string_of_defable typ)
  | _ -> "param" (*TODO*)

and string_of_param_list = function
  | [] -> ""
  | params -> Printf.sprintf "(%s)" (String.concat "," (List.map (string_of_param) params))

and string_of_let_modifier = function
  | Mut_Let -> "mut "
  | Normal_Let -> ""

and string_of_bare_definition = function
  | Def (name, param_list, ret_type, opt_body) ->
    Printf.sprintf "def %s %s" name (string_of_param_list param_list)

  | Let (let_modif, name, opt_typ, opt_body) ->
    Printf.sprintf "let %s%s" (string_of_let_modifier let_modif) name

and string_of_definition (pub,bare_defin,span) =
  (Printf.sprintf "%s%s" (if pub then "pub " else "") (string_of_bare_definition bare_defin))
