open Printf

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

and peek_span stream =
  match Stream.peek stream with
  | Some (_,span)-> span
  | None -> Span.span ((-1,0), 0) (*Someone else will complain about EOF*)

and parse_identifier = parser
                     | [< '(Token.Identifier ident,_) >] -> ident
                     | [< err=(error_expected "an identifier") >] -> raise err

and parse_defable = parser
                  | [< '(Token.Integer_Literal num,span) >] -> (Ast.Integer_Literal num,span)
                  | [< err=(error_expected "a defable") >] -> raise err

and parse_parameter_modifier = parser
                             | [< '(Token.Let,_) >] -> Ast.Let_Param
                             | [< '(Token.Mut,_) >] -> Ast.Mut_Param
                             | [< '(Token.Move,_) >] -> Ast.Move_Param
                             | [< '(Token.Copy,_) >] -> Ast.Copy_Param
                             | [< '(Token.Uncrt,_) >] -> Ast.Uncrt_Param
                             | [< '(Token.Dtor,_) >] -> Ast.Dtor_Param
                             | [< >] -> Ast.Normal_Param

and parse_parameter =
  parser
| [< start_span=peek_span;
     modif=parse_parameter_modifier;
     name=parse_identifier;
     _=expect_tok Token.Type_Separator;
     (typ, end_span)=parse_defable >]
  -> (Ast.Value_Param (modif,name,typ),Span.interval_of_spans start end_span)

and after_param_parse = parser
                      | [< '(Token.Right_Paren) >] -> []
                      | [< '(Token.Comma); params=parse_parameter_rec >] -> params
                      | [< err=error_expected "',' or ')'" >] -> err

and parse_parameter_rec = parser
                        | [< param = parse_parameter; params=after_param_parse >] -> param :: params

and parse_first_parameter = parser
                          | [< '(Tokens.Right_Paren,_) >] -> []
                          | [< params=parse_parameter_rec >] -> params

and parse_parameter_list = parser
                         | [< '(Token.Left_Paren,_); params=parse_first_parameter >] -> params
                         | [< >] -> [] (*No paramater list*)

and parse_return_modifier = parser
                          | [< '(Token.Let,_) >] -> Ast.Ref_Ret
                          | [< '(Token.Mut,_) >] -> Ast.Mut_Ref_Ret
                          | [< >] -> Ast.Value_Ret

and parse_type_opt stream = match Stream.peek stream with
  | Some (Token.Assign,_) -> None
  | Some (Token.Statement_End,_) -> None
  | _ -> Some parse_defable stream

and parse_return_type = parser
                      | [< '(Token.Type_Separator,_); modif=parse_return_modifier; typ=parse_type_opt >]
                        -> Some (modif,typ)
                      | [< >] -> None

and parse_def_body stream = match Stream.peek stream with
  | Some (Token.Statement_End,_) -> None
  | Some (Token.Assign,_) -> Stream.junk stream; Some parse_defable stream
  | _ -> Some parse_defable stream (*TODO Only allow scope when there is no = *)

and parse_def = parser
              | [< name=parse_identifier; params=parse_parameter_list; return=parse_return_type; body=parse_def_body >]

and parse_bare_definition : Ast.bare_definition =
  parser
| [< '(Token.Def, _); def=parse_def; >] -> def,start_span
| [< err=(error_expected "a definition") >] -> raise err

and is_pub = parser
           | [< '(Token.Pub, span) >] -> (true, span)
           | [< span=peek_span >] -> (false, span)

and parse_definition : Ast.definition =
  parser
| [< (pub, start)=is_pub; bare_defin=parse_bare_definition; end_span=expect_token Token.Statement_End >]
  -> (pub, bare_defin, Span.interval_of_spans start end_span)

and parse_all_definitions stream = match Stream.peek stream with
  | Some _ -> parse_definition stream :: parse_all_definitions stream
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
      Log.log_from_file_interval file_name (Span.interval_of_span span) Log.Fatal_Error
        (Printf.sprintf "expected %s before %s" expected (Token.token_to_string token));
      []
    | UnexpectedEOF expected ->
      Log.log_from_file file_name Log.Fatal_Error (Printf.sprintf "expected %s before EOF" expected);
      []
  with e ->
    close_in_noerr ic;
    raise e
