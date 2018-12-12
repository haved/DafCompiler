open Printf
open Span
module Ast=Daf_ast

(*
    ==== Error handling ====
*)

exception UnexpectedToken of Token.token_with_span * string
exception UnexpectedEOF of string

type stmt_or_result_expr = Statement of Ast.statement | Result_Expr of (Ast.defable*Span.loc_t)

let rec error_expected what = parser
                      | [< 'token_with_span >] -> UnexpectedToken (token_with_span, what)
                      | [< >] -> UnexpectedEOF what

and expect_tok expected = parser
                        | [< '(found, span) as token_with_span >] ->
                          if found = expected then span
                          else raise (UnexpectedToken(token_with_span, Token.token_to_string expected))
                        | [< >] -> raise (UnexpectedEOF (Token.token_to_string expected))

(*
     ==== Helper parsers ====
*)

and peek_span stream =
  match Stream.peek stream with
  | Some (_,span)-> span
  | None -> ((-1,0),(-1,0))

and parse_identifier = parser
                     | [< '(Token.Identifier ident,span) >] -> (ident,span)
                     | [< err=error_expected "an identifier" >] -> raise err (*TODO No langauge in error messages*)

(*
    ==== The defable, either an expression, a type or a namespace ====
*)

and parse_single_defable = parser
                         | [< '(Token.Identifier id,span) >] -> (Ast.Identifier id, span)

                         | [< '(Token.Integer_Literal num,span) >] -> (Ast.Integer_Literal num,span)
                         | [< '(Token.Def,start_span); (def_literal,end_span)=parse_def_literal_values >]
                           -> (def_literal, start_span<>end_span)
                         | [< '(Token.Scope_Start,(start_loc,_)); (stmts,result,end_loc)=parse_scope_contents >]
                           -> (Ast.Scope (stmts,result), Span.span start_loc end_loc)

                         | [< '(Token.Left_Paren,_); defable=parse_defable; _=expect_tok Token.Right_Paren >] -> defable

                         | [< '(Token.U8,   span) >] -> (Ast.Primitive_Type_Literal Ast.U8,   span)
                         | [< '(Token.I8,   span) >] -> (Ast.Primitive_Type_Literal Ast.I8,   span)
                         | [< '(Token.U16,  span) >] -> (Ast.Primitive_Type_Literal Ast.U16,  span)
                         | [< '(Token.I16,  span) >] -> (Ast.Primitive_Type_Literal Ast.I16,  span)
                         | [< '(Token.U32,  span) >] -> (Ast.Primitive_Type_Literal Ast.U32,  span)
                         | [< '(Token.I32,  span) >] -> (Ast.Primitive_Type_Literal Ast.I32,  span)
                         | [< '(Token.U64,  span) >] -> (Ast.Primitive_Type_Literal Ast.U64,  span)
                         | [< '(Token.I64,  span) >] -> (Ast.Primitive_Type_Literal Ast.I64,  span)
                         | [< '(Token.F32,  span) >] -> (Ast.Primitive_Type_Literal Ast.F32,  span)
                         | [< '(Token.F64,  span) >] -> (Ast.Primitive_Type_Literal Ast.F64,  span)
                         | [< '(Token.Usize,span) >] -> (Ast.Primitive_Type_Literal Ast.USIZE,span)
                         | [< '(Token.Isize,span) >] -> (Ast.Primitive_Type_Literal Ast.ISIZE,span)
                         | [< '(Token.Bool, span) >] -> (Ast.Primitive_Type_Literal Ast.BOOL, span)
                         | [< '(Token.Char, span) >] -> (Ast.Primitive_Type_Literal Ast.CHAR, span)

                         | [< err=error_expected "a defable">] -> raise err

and precedence_of_postfix_op = 2000

and precedence_of_prefix_op = function
  | _ -> 1000

and precedence_of_infix_op = function
  | Ast.Access_Op -> 2000
  | Ast.Mult | Ast.Divide | Ast.Mod -> 900
  | Ast.Plus | Ast.Minus -> 800
  | Ast.Left_Shift | Ast.Right_Shift | Ast.Logic_Right_Shift -> 700
  | Ast.Lower | Ast.LowerEq | Ast.Greater | Ast.GreaterEq -> 600
  | Ast.Equals | Ast.Not_Equals -> 500
  | Ast.Bitwise_And -> 280
  | Ast.Bitwise_Xor -> 260
  | Ast.Bitwise_Or -> 240
  | Ast.Logical_And -> 220
  | Ast.Logical_Or -> 200
  | Ast.Assignment -> 100

and is_infix_op_left_to_right = function
  | Ast.Assignment -> false
  | _ -> true

and parse_prefix_op_opt =
  let parse_ref_augment span = parser
                           | [< '(Token.Mut, span2) >] -> (Ast.MutRef, span<>span2)
                           | [< >] -> (Ast.Ref, span)
  in parser
   | [< '(Token.Ref, span); result=parse_ref_augment span >] -> Some result
   | [< '(Token.Plus,  span) >] -> Some (Ast.Positive, span)
   | [< '(Token.Minus, span) >] -> Some (Ast.Negative, span)
   | [< '(Token.Not, span) >] -> Some (Ast.Not, span)
   | [< '(Token.Bitwise_Not, span) >] -> Some (Ast.Bitwise_Not, span)
   | [< '(Token.Plus_Plus,   span) >] -> Some (Ast.Pre_Increase, span)
   | [< '(Token.Minus_Minus, span) >] -> Some (Ast.Pre_Decrease, span)
   | [< '(Token.Dereference, span) >] -> Some (Ast.Dereference, span)
   | [< '(Token.Sizeof, span) >] -> Some (Ast.Sizeof, span)
   | [< >] -> None

and parse_argument_modifier = parser
                            | [< '(Token.Mut,  span) >] -> (Ast.Arg_Mut,  span)
                            | [< '(Token.Move, span) >] -> (Ast.Arg_Move, span)
                            | [< '(Token.Copy, span) >] -> (Ast.Arg_Copy, span)
                            | [< '(Token.Uncrt, span) >] -> (Ast.Arg_Uncrt, span)
                            | [< span=peek_span >] -> (Ast.Arg_Normal, span)

and parse_arg stream =
  (
    let (modif,start_span) = parse_argument_modifier stream in
    let (_,end_span)as defable=parse_defable stream in
    (modif, defable, start_span<>end_span)
  )

and parse_post_arg_in_list = parser
  | [< '(Token.Comma,_); args=parse_arg_then_list >] -> args
  | [< '(Token.Right_Paren,(_,end_loc)) >] -> ([], end_loc)
  | [< err=error_expected ") or ," >] -> raise err (* TODO: Don't have string literals for error messages spread about *)

and parse_arg_then_list = parser
  [< arg = parse_arg ; (rest,end_loc) = parse_post_arg_in_list >] -> (arg::rest, end_loc)

and parse_arg_list = parser
                   | [< '(Token.Right_Paren, (_,end_loc)) >] -> ([],end_loc)
                   | [< args = parse_arg_then_list >] -> args

and parse_postfix_op_opt = parser
                        | [< '(Token.Plus_Plus,   span) >] -> Some (Ast.Post_Increase, span)
                        | [< '(Token.Minus_Minus, span) >] -> Some (Ast.Post_Decrease, span)
                        | [< '(Token.Left_Paren, (start_loc,_)); (arg_list,end_loc)=parse_arg_list >]
                          -> Some (Ast.Function_Call arg_list, Span.span start_loc end_loc)
                        | [< '(Token.Left_Bracket, start_span); index=parse_defable;
                           end_span=expect_tok Token.Right_Bracket >] ->
                           Some(Ast.Array_Access index, start_span<>end_span)
                        | [< >] -> None

and parse_postfix_ops operand min_precedence stream =
  if precedence_of_postfix_op < min_precedence then
    operand
  else match parse_postfix_op_opt stream with
       | None -> operand
       | Some (op, op_span) ->
          let expr = (Ast.Postfix_Operator (op,operand), (Util.scnd operand)<>op_span) in
          parse_postfix_ops expr precedence_of_postfix_op stream

and infix_op_of_token_opt = function
  | Token.Class_Access -> Some Ast.Access_Op
  | Token.Mult -> Some Ast.Mult | Token.Divide -> Some Ast.Divide | Token.Modulo -> Some Ast.Mod
  | Token.Plus -> Some Ast.Plus | Token.Minus -> Some Ast.Minus
  | Token.Lsl -> Some Ast.Left_Shift | Token.Asr -> Some Ast.Right_Shift | Token.Lsr -> Some Ast.Logic_Right_Shift
  | Token.Lower -> Some Ast.Lower | Token.Lower_Or_Equal -> Some Ast.LowerEq
  | Token.Greater -> Some Ast.Greater | Token.Greater_Or_Equal -> Some Ast.GreaterEq
  | Token.Equals -> Some Ast.Equals | Token.Not_Equals -> Some Ast.Not_Equals
  | Token.Ref -> Some Ast.Bitwise_And
  | Token.Bitwise_Xor -> Some Ast.Bitwise_Xor
  | Token.Bitwise_Or -> Some Ast.Bitwise_Or
  | Token.Logical_And -> Some Ast.Logical_And
  | Token.Logical_Or -> Some Ast.Logical_Or
  | Token.Assign -> Some Ast.Assignment
  | _ -> None

and parse_infix_ops lhs min_precedence stream =
  match Stream.peek stream with
  | None -> lhs
  | Some (tok, op_span) ->
    match infix_op_of_token_opt tok with
    | None -> lhs
    | Some op -> let prec = precedence_of_infix_op op in
      if prec < min_precedence then lhs (*The precedence of the op is too low, pop the call stack until it isn't*)
      else (
        Stream.junk stream;
        let left_to_right = is_infix_op_left_to_right op in
        let rhs = parse_defables (prec + if left_to_right then 1 else 0) stream in
        let combined = (Ast.Infix_Operator (op, lhs, rhs), (lhs|>Util.scnd)<>(rhs|>Util.scnd)) in
        parse_infix_ops combined min_precedence stream
      )


and parse_defables min_precedence stream : (Ast.defable)=
  (let pre = match parse_prefix_op_opt stream with
     | None -> parse_single_defable stream
     | Some (op, op_span) ->
        let operand = parse_defables (precedence_of_prefix_op op) stream in
        (Ast.Prefix_Operator (op, operand), op_span<>(Util.scnd operand))
   in let pre_in = parse_infix_ops pre min_precedence stream
   in let pre_in_post = parse_postfix_ops pre_in min_precedence stream
   in pre_in_post
  )


and parse_defable stream = parse_defables 0 stream

(*
   ==== Types ====
*)



(*
   ==== Scopes and statements ====
*)

and parse_scope = parser
                | [< '(Token.Scope_Start,(start_loc,_)); (stmts,result,end_loc)=parse_scope_contents >]
                  -> (Ast.Scope (stmts,result), Span.span start_loc end_loc)
                | [< err=error_expected "a scope" >] -> raise err

and parse_scope_contents = parser
                         | [< '(Token.Scope_End,(_,end_loc)) >] -> ([],None,end_loc)
                         | [< stmt_or_result=parse_statement_or_result_expr; stream >]
                           -> match stmt_or_result with
                           | Statement stmt ->
                             let (rest,result,end_loc)=parse_scope_contents stream in
                             (stmt::rest, result, end_loc)
                           | Result_Expr (result,end_loc) ->
                             ([], Some result, end_loc)

and parse_else_opt = parser
                        | [< '(Token.Else,_); body=parse_statement >] -> Some body (* to love *)
                        | [< >] -> None

and parse_packed_statement_or_result_expr stream =
  match Stream.peek stream with
  | None -> raise (error_expected "a statement" stream)
  | Some (token,start_span) -> match token with
    | Token.Statement_End ->
      Stream.junk stream;
      Statement (Ast.NopStatement, start_span)
    | Token.Def | Token.Let | Token.Typedef ->
      let defin = parse_bare_definition stream in
      let end_span = expect_tok Token.Statement_End stream in
      Statement (Ast.DefinitionStatement defin, start_span<>end_span)
    | _ -> let expression = parse_defable stream in
      stream |> parser
        | [< '(Token.Scope_End,(_,end_loc)) >] -> Result_Expr (expression, end_loc)
        | [< >] ->
          match expression with
          | (Ast.Scope _, _) -> Statement (Ast.ExpressionStatement expression, Util.scnd expression)
          | _ -> let semicolon_span = expect_tok Token.Statement_End stream in
          Statement (Ast.ExpressionStatement expression, start_span<>semicolon_span)

and parse_statement_or_result_expr =
  parser
| [< '(Token.If, start); cond=parse_defable; (_,body_end) as body=parse_statement; else_opt=parse_else_opt >]
  -> let end_span = (match else_opt with Some (_,e)->e | None -> body_end) in
  Statement (Ast.If (cond, body, else_opt), start<>end_span)
| [< stmt_or_re = parse_packed_statement_or_result_expr >] -> stmt_or_re

and parse_statement stream = match parse_statement_or_result_expr stream with
  | Statement stmt -> stmt
  | Result_Expr (_,_) -> raise (error_expected "a statement, not a result value" stream)

(*
    ==== Everything related to def_literal, also used by def ====
*)

and parse_parameter_modifier = parser
                             | [< '(Token.Let,_) >] -> Ast.Let_Param
                             | [< '(Token.Mut,_) >] -> Ast.Mut_Param
                             | [< '(Token.Move,_) >] -> Ast.Move_Param
                             | [< '(Token.Copy,_) >] -> Ast.Copy_Param
                             | [< '(Token.Uncrt,_) >] -> Ast.Uncrt_Param
                             | [< '(Token.Dtor,_) >] -> Ast.Dtor_Param
                             | [< >] -> Ast.Normal_Param

and parse_def_parameter = parser
                        | [< '(Token.Def,start_span); (def_param,end_span) = parse_parameter_def_values >]
                          -> (def_param, span_over start_span end_span)
                        | [< start_span=peek_span; modif=parse_parameter_modifier; (name,_)=parse_identifier;
                           _=expect_tok Token.Type_Separator; typ=parse_defable >]
                          -> (Ast.Value_Param (modif,name,typ), start_span<>(typ|>Util.scnd))

and parse_post_def_param_in_list = parser
  | [< '(Token.Right_Paren,end_span) >] -> ([],end_span)
  | [< '(Token.Comma,_); params=parse_def_param_then_list >] -> params
  | [< err=error_expected "',' or ')'" >] -> raise err

and parse_def_param_then_list = parser
                              | [< param = parse_def_parameter; (rest,end_span) = parse_post_def_param_in_list >]
                                -> (param::rest, end_span)

and parse_first_def_parameter = parser
                          | [< '(Token.Right_Paren,end_span) >] -> ([],end_span)
                          | [< params=parse_def_param_then_list >] -> params

and parse_def_parameter_list = parser
                             | [< '(Token.Left_Paren,start_span); (params,end_span)=parse_first_def_parameter >]
                               -> (params, Some(start_span<>end_span))
                             | [< >] -> ([], None) (*empty paramater list, no span*)

and parse_def_return_modifier = parser
                          | [< '(Token.Let,span) >] -> (Ast.Ref_Ret,Some span)
                          | [< '(Token.Mut,span) >] -> (Ast.Mut_Ref_Ret,Some span)
                          | [< >] -> (Ast.Value_Ret,None)

and parse_type_or_inferred stream = match Stream.peek stream with
  | Some (Token.Assign,_) -> None (*We don't eat this, as it is part of def body parsing*)
  | _ -> Some (parse_defable stream)

and parse_def_return_type = parser
                          | [< '(Token.Type_Separator,start_span);
                             (modif,modif_s)=parse_def_return_modifier; (typ,typ_s)=parse_type_or_inferred >]
                        -> Some (modif,typ,start_span<>(Util.first_some [typ_s ; modif_s] start_span))
                      | [< >] -> None

and parse_def_body stream = match Stream.peek stream with
  | Some (Token.Assign,_) -> Stream.junk stream; parse_defable stream
  | _ -> parse_scope stream (* If there is no '=', we only allow a scope body *)

and parse_def_literal_values = parser
                      | [< start=peek_span; (params,_)=parse_def_parameter_list; return=parse_def_return_type; body=parse_def_body >]
                        -> (Ast.Def_Literal (params, return, body), start<>(Util.scnd body))

and parse_def_values = parser
              | [< (name,_)=parse_identifier; (params,_)=parse_def_parameter_list; (return,_)=parse_def_return_type; body=parse_optional_def_body >]
                -> Ast.Def (name, params, return, body)

and parse_parameter_def_values = parser
                               | [< (name,n_span)=parse_identifier; (params,p_span)=parse_def_parameter_list;
                                  return=parse_def_return_type >]
                                 -> (Ast.Def_Param (name, params, return),
                                     n_span<>Util.first_some [Option.map Util.thrd return ; p_span] n_span)

(*
    ==== All definitions ====
    They are first parsed "bare", meaning without span info or the 'pub' keyword or trailing semicolon.
*)

and parse_optional_def_body stream = match Stream.peek stream with
  | Some (Token.Statement_End,_) -> None
  | _ -> Some (parse_def_body stream)

and parse_let_modifiers = parser
                        | [< '(Token.Mut,_) >] -> Ast.Mut_Let
                        | [< >] -> Ast.Normal_Let

and parse_optional_let_type_and_assignment =
  let parse_optional_body = parser
                          | [< '(Token.Assign,_); body=parse_defable >] -> Some body
                          | [< >] -> None
  in parser
   | [< '(Token.Assign,_); body=parse_defable >] -> (None,Some body)
   | [< typ=parse_defable; body=parse_optional_body >] -> (Some typ,body)

and parse_let_values = parser
                     | [< modif=parse_let_modifiers; (name,_)=parse_identifier; '(Token.Type_Separator,_); (typ,assign)=parse_optional_let_type_and_assignment >] -> Ast.Let (modif, name, typ, assign)

and parse_bare_definition = parser
                          | [< '(Token.Def,_); def=parse_def_values; >] -> def
                          | [< '(Token.Let,_); let_values=parse_let_values; >] -> let_values
                          | [< err=(error_expected "a definition") >] -> raise err

(* Turning bare definitions into definitions *)

and parse_is_pub = parser
                 | [< '(Token.Pub, start_span) >] -> (true, start_span)
                 | [< start_span=peek_span >] -> (false, start_span)

and parse_definition = parser
                     | [< (pub, start_span)=parse_is_pub; bare_defin=parse_bare_definition; end_span=expect_tok Token.Statement_End >]
                       -> (pub, bare_defin, start_span<>end_span)

(*
   ==== File parsing code ====
*)

let rec parse_all_definitions stream = match Stream.peek stream with
  | Some (tok,_) ->
    let defin = parse_definition stream in
    defin :: parse_all_definitions stream
  | None -> []

let definition_list_of_file file_name =
  let ic = open_in file_name in
  try
    try
      let stream = Lexer.lex (Stream.of_channel ic) in
      let defins = parse_all_definitions stream in
      close_in ic;
      defins
    with
    | UnexpectedToken ((token, span), expected) ->
      Log.log_from_file_span file_name span Log.Fatal_Error
        (sprintf "expected %s before %s" expected (Token.token_to_string token));
      []
    | UnexpectedEOF expected ->
      Log.log_from_file file_name Log.Fatal_Error (sprintf "expected %s before EOF" expected);
      []
  with e ->
    close_in_noerr ic;
    raise e
