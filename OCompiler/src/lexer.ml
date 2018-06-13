
let rec lex_singles = parser
            | [< ' (' ' | '\r' | '\n' | '\t'); stream >] -> lex_singles stream

            | [< ' ('A' .. 'Z' | 'a' .. 'z' as c); stream >] ->
              let buffer = Buffer.create 1 in
              Buffer.add_char buffer c;
              lex_ident buffer stream

            | [< ' ('0' .. '9' as c); stream >] ->
              let buffer = Buffer.create 1 in
              Buffer.add_char buffer c;
              lex_number buffer stream

            | [< ' ('/'); stream >] ->
              lex_line_comment_first stream

            | [< 'c; stream >] ->
              [< ' Token.char_to_token c; lex_singles stream >]

            (*End of stream*)
            | [< >] -> [< >]

and lex_ident buffer = parser
                     | [< ' ('A' .. 'Z' | 'a' .. 'z' | '0' .. '9' | '_' as c); stream >] ->
                       Buffer.add_char buffer c;
                       lex_ident buffer stream
                     | [< stream=lex_singles >] ->
                       match Buffer.contents buffer with
                       | id -> [< 'Token.Identifier id; stream >]

and lex_number buffer = parser
                      | [< ' ('0' .. '9' | '.' as c); stream >] ->
                        Buffer.add_char buffer c;
                        lex_number buffer stream
                      | [< stream=lex_singles >] ->
                        [< 'Token.Real_Literal (float_of_string (Buffer.contents buffer)); stream >]

and lex_line_comment_first = parser
                | [< ' ('/'); stream >] -> lex_line_comment stream
                | [< stream=lex_singles >] -> [< ' (Token.char_to_token '/'); stream >]

and lex_line_comment = parser
                | [< ' ('\n'); stream=lex_singles >] -> stream
                | [< 'c; e=lex_line_comment >] -> e
                | [< >] -> [< >]

let lex char_stream =
  let token_stream = lex_singles char_stream in
  Stream.from
    (fun _ ->
       try
         Some (Stream.next token_stream)
       with
       | Stream.Failure -> None (*Means end of stream*)
       | e -> raise e (*All other errors shall be raised*)
    )
