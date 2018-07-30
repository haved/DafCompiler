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
                     | [< err=(error_expected "an identifier") >] -> raise err

(*
    ==== The defable, either an expression, a type or a namespace ====
*)

and parse_defable = parser
                  | [< '(Token.Integer_Literal num,span) >] -> (Ast.Integer_Literal num,span)
                  | [< '(Token.Def,start); (def_literal,end_span)=parse_def_literal >]
                    -> (def_literal, Span.span_over start end_span)
                  | [< err=(error_expected "a defable") >] -> raise err

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
  | _ -> parse_defable stream (* TODO there is no '=' => only allow scope *)

and parse_def_literal = parser
                      | [< start=peek_span; params=parse_parameter_list; return=parse_return_type; (_,end_span)as body=parse_def_body >]
                        -> (Ast.Def_Literal (params, return, body), Span.span_over start end_span)

(*
    ==== All definitions ====
    They are first parsed "bare", meaning without span info or the 'pub' keyword.
*)

and parse_optional_def_body stream = match Stream.peek stream with
  | Some (Token.Statement_End,_) -> None
  | _ -> Some (parse_def_body stream)

and parse_def = parser
              | [< name=parse_identifier; params=parse_parameter_list; return=parse_return_type; body=parse_optional_def_body >]
                -> Ast.Def (name, params, return, body)

and parse_bare_definition = parser
                          | [< '(Token.Def,_); def=parse_def; >] -> def
                          | [< err=(error_expected "a definition") >] -> raise err

and parse_is_pub = parser
                 | [< '(Token.Pub, span) >] -> (true, span)
                 | [< span=peek_span >] -> (false, span)

and parse_definition = parser
                     | [< (pub, start)=parse_is_pub; bare_defin=parse_bare_definition; end_span=expect_tok Token.Statement_End >]
                       -> (pub, bare_defin, Span.span_over start end_span)

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