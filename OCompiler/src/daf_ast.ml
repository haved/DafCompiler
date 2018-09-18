
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

let rec string_of_defable tab (bare_defable,_) = match bare_defable with
  | Identifier id -> id
  | Integer_Literal int -> Printf.sprintf "%d" int
  | Real_Literal float -> Printf.sprintf "%f" float
  | _ -> "defable"

and string_of_parameter_modifier = function
  | Normal_Param -> ""
  | Let_Param -> "let "
  | Mut_Param -> "mut "
  | Move_Param -> "move "
  | Copy_Param -> "copy "
  | Uncrt_Param -> "uncrt "
  | Dtor_Param -> "dtor "

and string_of_param tab (bare_param, _) = match bare_param with
  | Value_Param (param_modif, name, typ) ->
    Printf.sprintf "%s%s:%s" (string_of_parameter_modifier param_modif) name (string_of_defable tab typ)
  | _ -> "param" (*TODO*)

and string_of_param_list tab = function
  | [] -> ""
  | params -> Printf.sprintf "(%s)" (String.concat ", " (List.map (string_of_param tab) params))

and string_of_return_modifier = function
  | Value_Ret -> ""
  | Ref_Ret -> "let "
  | Mut_Ref_Ret -> "mut "

and string_of_return_type tab = function
  | None -> ""
  | Some (ret_modif, opt_typ) ->
    Printf.sprintf ":%s%s" (string_of_return_modifier ret_modif)
      (match opt_typ with None -> "" | Some typ -> string_of_defable (tab+2) typ)

and string_of_let_modifier = function
  | Mut_Let -> "mut "
  | Normal_Let -> ""

and string_of_opt_body tab = function
  | None -> ""
  | Some body -> Printf.sprintf "=%s" (string_of_defable tab body)

and string_of_bare_definition tab = function
  | Def (name, param_list, ret_type, opt_body) ->
    Printf.sprintf "def %s%s%s%s" name (string_of_param_list tab param_list)
      (string_of_return_type tab ret_type) (string_of_opt_body tab opt_body)
  | Let (let_modif, name, opt_typ, opt_body) ->
    Printf.sprintf "let %s%s:%s%s" (string_of_let_modifier let_modif) name
      (match opt_typ with None -> "" | Some typ -> (string_of_defable tab typ))
      (string_of_opt_body tab opt_body)

and string_of_definition tab (pub,bare_defin,span) =
  (Printf.sprintf "%*s%s%s" tab "" (if pub then "pub " else "") (string_of_bare_definition tab bare_defin))
