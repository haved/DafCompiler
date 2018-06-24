open Printf

exception UnexpectedToken of Token.token_with_span * string
exception UnexpectedEOF of string

let rec expected what = parser
                      | [< 'token_with_span >] -> UnexpectedToken (token_with_span, what)
                      | [< >] -> UnexpectedEOF what

and parse_expression = parser
                        | [< ' (Token.Integer_Literal num, span) >] -> (Ast.Integer_Literal num,span)
                        | [< err=(expected "an expression") >] -> raise err

and parse_definition = parser
                     | [< ' (Token.Def, loc) >] -> Ast.Minus
                     | [< err=(expected "a definition") >] -> raise err

and parse_all_definitions list = parser
                               | [< >] -> list
                               | [< defin = parse_definition; rest = parse_all_definitions >] -> [defin :: rest]

let file_to_definition_list file_name =
  let ic = open_in file_name in
  try
    try
      let stream = Lexer.lex (Stream.of_channel ic) in
      let defins = parse_all_definitions [] stream in
      close_in ic;
      defins
    with
    | UnexpectedToken ((token, span), expected) as e ->
      Log.log_from_file_interval file_name (Span.interval_of_span span) Log.Fatal_Error
        (Printf.sprintf "expected %s before %s" expected (Token.token_to_string token));
      raise e
    | UnexpectedEOF expected as e ->
      Log.log_from_file file_name Log.Fatal_Error (Printf.sprintf "expected %s before EOF" expected);
      raise e
  with e ->
    close_in_noerr ic;
    raise e
