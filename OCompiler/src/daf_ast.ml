
(* ==== Defables ==== *)

type bare_defable =
  | Identifier of string

  | Integer_Literal of int
  | Real_Literal of float
  | Def_Literal of parameter list * return_type * defable
  | Scope of statement list * defable option

  | Primitive_Type_Literal of primitive_type

  | Infix_Operator of infix_operator * defable * defable
  | Prefix_Operator of prefix_operator * defable
  | Postfix_Operator of postfix_operator * defable

and defable = bare_defable * Span.span_t

(* ==== Operators ==== *)

and infix_operator =
  | Plus | Minus | Mult | Divide | Access_Operator

and prefix_operator =
  | Ref | MutRef | Pre_Increase | Pre_Decrease

and argument_modifier = Arg_Normal | Arg_Mut | Arg_Move | Arg_Copy | Arg_Uncrt

and argument = argument_modifier * defable * Span.span_t

and postfix_operator =
  | Post_Increase | Post_Decrease | Array_Access of defable | Function_Call of argument list

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

open Printf

let rec string_of_defable tab (bare_defable,_) = match bare_defable with
  | Identifier id -> id
  | Integer_Literal int -> Printf.sprintf "%d" int
  | Real_Literal float -> Printf.sprintf "%f" float
  | Scope (statement_list, result) ->
    Printf.sprintf "{\n%s%s%*s}" (string_of_statement_list (tab+2) statement_list)
                                 (string_of_opt_result_expr (tab+2) result) tab ""
  | Primitive_Type_Literal primitive_type -> string_of_primitive_type primitive_type
  | Infix_Operator (op, lhs, rhs) ->
    Printf.sprintf "%s %s %s" (string_of_defable (tab+2) lhs) (string_of_infix_operator op) (string_of_defable (tab+2) rhs)
  | Prefix_Operator (op, rhs) ->
    Printf.sprintf "%s%s" (string_of_prefix_operator op) (string_of_defable (tab+2) rhs)
  | Postfix_Operator (op, lhs) ->
    Printf.sprintf "%s%s" (string_of_defable (tab+2) lhs) (string_of_postfix_operator tab op)
  | _ -> "defable"

and string_of_infix_operator defin = match defin with
  | Plus -> "+"
  | Minus -> "-"
  | Mult -> "*"
  | Divide -> "/"
  | Access_Operator -> "."

and string_of_prefix_operator = function
  | Ref -> "&"
  | MutRef -> "&mut "
  | Pre_Increase -> "++"
  | Pre_Decrease -> "--"

and string_of_argument tab arg =
  "arg"

and string_of_argument_list tab list =
  String.concat ", " (List.map (string_of_argument tab) list)

and string_of_postfix_operator tab = function
  | Post_Increase -> "++"
  | Post_Decrease -> "--"
  | Array_Access index -> Printf.sprintf "[ %s ]" (string_of_defable (tab+2) index)
  | Function_Call arg_list -> Printf.sprintf "(%s)" (string_of_argument_list (tab+2) arg_list)

and string_of_primitive_type = function
  | U8 -> "u8" | I8 -> "i8"
  | U16 -> "u16" | I16 -> "i16"
  | U32 -> "u32" | I32 -> "i32"
  | U64 -> "u64" | I64 -> "i64"
  | F32 -> "f32" | F64 -> "f64"
  | USIZE -> "usize" | ISIZE -> "isize"
  | BOOL -> "bool" | CHAR -> "char"

and string_of_statement tab (bare_stmt, _) = match bare_stmt with
  | NopStatement -> ";"
  | ExpressionStatement defable -> Printf.sprintf "%s;" (string_of_defable tab defable)
  | If (cond,body,else_opt) -> Printf.sprintf "if %s %s" (string_of_defable tab cond) (string_of_statement tab body)
  | _ -> "statement" (*TODO*)

and string_of_statement_list tab list =
  match list with
  | head :: rest -> Printf.sprintf "%*s%s\n%s" tab ""
                      (string_of_statement tab head)
                      (string_of_statement_list tab rest)
  | [] -> ""

and string_of_opt_result_expr tab = function
  | Some expr -> Printf.sprintf "%*s%s\n" tab "" (string_of_defable tab expr)
  | None -> ""

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
