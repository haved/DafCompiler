open Printf

exception UnexpectedToken of Token.token_with_span * string
exception UnexpectedEOF of string

let rec error_expected what = parser
                      | [< 'token_with_span >] -> UnexpectedToken (token_with_span, what)
                      | [< >] -> UnexpectedEOF what

and expect_tok expected = parser
                        | [< '(found, span) as token_with_span >] ->
                          if found = expected then found
                          else raise (UnexpectedToken(token_with_span, Token.token_to_string expected))
                        | [< >] -> raise (UnexpectedEOF (Token.token_to_string expected))

and parse_defable = parser
                        | [< ' (Token.Integer_Literal num, span) >] -> (Ast.Integer_Literal num,span)
                        | [< err=(error_expected "an expression") >] -> raise err

and parse_def = parser
              | [<>] -> None

and parse_definition (pub:bool) : Ast.Definition = parser
                     | [< ' (Token.Def, loc); def=parse_def >] -> def
                     | [< err=(error_expected "a definition") >] -> raise err

and parse_all_definitions = parser
                               | [< defin = parse_definition; rest = parse_all_definitions >] -> defin :: rest
                               | [< >] -> []

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
