open Printf

(*
    ==== Error handeling ====
*)

exception UnexpectedToken of Token.token_with_span * string
exception UnexpectedEOF of string

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
                         | [< '(Token.Scope_Start,(start_loc,_)); (scope,end_loc)=parse_scope_contents >]
                           -> (Ast.Scope scope, Span.span start_loc end_loc)

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

                         | [< err=error_expected "a defable" >] -> raise err

(* === Parses until an operator with lower precedence is found *)
and parse_defables min_precedence stream = 0

and parse_defable = parse_defables 0

(*
   ==== Types ====
*)

(*
   ==== Scopes and statements ====
*)

and parse_scope = parser
                | [< '(Token.Scope_Start,(start_loc,_)); (scope,end_loc)=parse_scope_contents >]
                  -> (Ast.Scope scope, Span.span start_loc end_loc)
                | [< err=error_expected "a scope" >] -> raise err

and parse_scope_contents = parser
                      | [< '(Token.Scope_End,(_,end_loc)) >] -> ([],end_loc)
                      | [< stmt=parse_statement; (rest,end_loc)=parse_scope_contents >] -> (stmt::rest, end_loc)

and parse_else_opt = parser
                        | [< '(Token.Else,_); body=parse_statement >] -> Some body (* to love *)
                        | [< >] -> None

and parse_packed_statement stream =
  match Stream.peek stream with
  | None -> raise (error_expected "a statement" stream)
  | Some (token,start_span) -> match token with
    | Token.Scope_Start ->
      let (_,span) as scope = parse_scope stream in
      (Ast.ExpressionStatement scope,span)
    | Token.Def | Token.Let | Token.Typedef ->
      let defin = parse_bare_definition stream in
      let end_span = expect_tok Token.Statement_End stream in
      (Ast.DefinitionStatement defin, Span.span_over start_span end_span)
    | _ ->
      let expression = parse_defable   stream in
      let end_span  = expect_tok Token.Statement_End stream in
      (Ast.ExpressionStatement expression, Span.span_over start_span end_span)

and parse_statement =
  parser
| [< '(Token.If, start); cond=parse_defable; (_,body_end) as body=parse_statement; else_opt=parse_else_opt >]
  -> let end_span = (match else_opt with Some (_,e)->e | None -> body_end) in
  (Ast.If (cond, body, else_opt), Span.span_over start end_span)
| [< stmt = parse_packed_statement >] -> stmt

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
                         _=expect_tok Token.Type_Separator; (_,end_span) as typ=parse_defable >]
                      -> (Ast.Value_Param (modif,name,typ), Span.span_over start_span end_span)

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
                      | [< start=peek_span; params=parse_parameter_list; return=parse_return_type; (_,end_span)as body=parse_def_body >]
                        -> (Ast.Def_Literal (params, return, body), Span.span_over start end_span)

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

and parse_all_definitions stream = match Stream.peek stream with
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
