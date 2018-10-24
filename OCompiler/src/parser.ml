open Printf
module Ast=Daf_ast

(*
    ==== Error handeling ====
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
                     | [< '(Token.Identifier ident,_) >] -> ident
                     | [< err=error_expected "an identifier" >] -> raise err

(*
    ==== The defable, either an expression, a type or a namespace ====
*)

and parse_single_defable = parser
                         | [< '(Token.Identifier id,span) >] -> (Ast.Identifier id, span)

                         | [< '(Token.Integer_Literal num,span) >] -> (Ast.Integer_Literal num,span)
                         | [< '(Token.Def,start_span); (def_literal,end_span)=parse_def_literal_values >]
                           -> (def_literal, Span.span_over start_span end_span)
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

and precedence_of_postfix_op = function
  | Ast.Post_Increase | Ast.Post_Decrease
  | Ast.Function_Call _ | Ast.Array_Access _ -> 2000

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
                           | [< '(Token.Mut, span2) >] -> (Ast.MutRef, Span.span_over span span2)
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

and parse_postfix_ops operand min_precedence stream =
  match Stream.peek stream with
  | None -> operand
  | Some (tok, op_span) -> (
    let op = match tok with
      | Token.Plus_Plus -> Some Ast.Post_Increase
      | Token.Minus_Minus -> Some Ast.Post_Decrease
      | _ -> None
    in match op with
    | None -> operand
    | Some op -> let op_prec = precedence_of_postfix_op op in
      if (op_prec < min_precedence) then operand else
        let _,start_span = operand in
        Stream.junk stream;
        let expr = (Ast.Postfix_Operator (op,operand), Span.span_over start_span op_span) in
        parse_postfix_ops expr op_prec stream
  )

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
        let combined = (Ast.Infix_Operator (op, lhs, rhs), Span.span_over (lhs|>Span.from) rhs|>Span.from) in
        parse_infix_ops combined min_precedence stream
      )


and parse_defables min_precedence stream : (Ast.defable)=
  let pre = match parse_prefix_op_opt stream with
    | None -> parse_single_defable stream
    | Some (op, op_span) ->
      let operand = parse_defables (precedence_of_prefix_op op) stream in
      (Ast.Prefix_Operator (op, operand), Span.span_over op_span (Span.from operand))
  in let prepost = parse_postfix_ops pre min_precedence stream
  in let expr_down_to_min_prec = parse_infix_ops prepost min_precedence stream in
  expr_down_to_min_prec

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
      Statement (Ast.DefinitionStatement defin, Span.span_over start_span end_span)
    | _ -> let expression = parse_defable stream in
      stream |> parser
        | [< '(Token.Scope_End,(_,end_loc)) >] -> Result_Expr (expression, end_loc)
        | [< >] ->
          match expression with
          | (Ast.Scope _, _) -> Statement (Ast.ExpressionStatement expression, Span.from expression)
          | _ -> let semicolon_span = expect_tok Token.Statement_End stream in
          Statement (Ast.ExpressionStatement expression, Span.span_over start_span semicolon_span)

and parse_statement_or_result_expr =
  parser
| [< '(Token.If, start); cond=parse_defable; (_,body_end) as body=parse_statement; else_opt=parse_else_opt >]
  -> let end_span = (match else_opt with Some (_,e)->e | None -> body_end) in
  Statement (Ast.If (cond, body, else_opt), Span.span_over start end_span)
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

and parse_parameter = parser
                    | [< start_span=peek_span; modif=parse_parameter_modifier; name=parse_identifier;
                         _=expect_tok Token.Type_Separator; typ=parse_defable >]
                      -> (Ast.Value_Param (modif,name,typ), Span.span_over start_span typ|>Span.from)

and after_param_parse = parser
                      | [< '(Token.Right_Paren,_) >] -> []
                      | [< '(Token.Comma,_); params=parse_parameter_rec >] -> params
                      | [< err=error_expected "',' or ')'" >] -> raise err

and parse_parameter_rec = parser
                        | [< param=parse_parameter; params=after_param_parse >] -> param :: params

and parse_first_parameter = parser
                          | [< '(Token.Right_Paren,_) >] -> []
                          | [< params=parse_parameter_rec >] -> params

and parse_parameter_list = parser
                         | [< '(Token.Left_Paren,_); params=parse_first_parameter >] -> params
                         | [< >] -> [] (*No paramater list*)

and parse_return_modifier = parser
                          | [< '(Token.Let,_) >] -> Ast.Ref_Ret
                          | [< '(Token.Mut,_) >] -> Ast.Mut_Ref_Ret
                          | [< >] -> Ast.Value_Ret

and parse_type_or_inferred stream = match Stream.peek stream with
  | Some (Token.Assign,_) -> None
  | _ -> Some (parse_defable stream)

and parse_return_type = parser
                      | [< '(Token.Type_Separator,_); modif=parse_return_modifier; typ=parse_type_or_inferred >]
                        -> Some (modif,typ)
                      | [< >] -> None

and parse_def_body stream = match Stream.peek stream with
  | Some (Token.Assign,_) -> Stream.junk stream; parse_defable stream
  | _ -> parse_scope stream (* If there is no '=', we only allow a scope body *)

and parse_def_literal_values = parser
                      | [< start=peek_span; params=parse_parameter_list; return=parse_return_type; body=parse_def_body >]
                        -> (Ast.Def_Literal (params, return, body), Span.span_over start body|>Span.from)

(*
    ==== All definitions ====
    They are first parsed "bare", meaning without span info or the 'pub' keyword or trailing semicolon.
*)

and parse_optional_def_body stream = match Stream.peek stream with
  | Some (Token.Statement_End,_) -> None
  | _ -> Some (parse_def_body stream)

and parse_def_values = parser
              | [< name=parse_identifier; params=parse_parameter_list; return=parse_return_type; body=parse_optional_def_body >]
                -> Ast.Def (name, params, return, body)

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
                     | [< modif=parse_let_modifiers; name=parse_identifier; '(Token.Type_Separator,_); (typ,assign)=parse_optional_let_type_and_assignment >] -> Ast.Let (modif, name, typ, assign)

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
                       -> (pub, bare_defin, Span.span_over start_span end_span)

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
        (Printf.sprintf "expected %s before %s" expected (Token.token_to_string token));
      []
    | UnexpectedEOF expected ->
      Log.log_from_file file_name Log.Fatal_Error (Printf.sprintf "expected %s before EOF" expected);
      []
  with e ->
    close_in_noerr ic;
    raise e
