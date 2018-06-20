
(* These are identical:
   [< next_parser = lex_singles >] -> next_parser
   [< next_parser = lex_singles >] -> [< next_parser >]
   [< stream >] -> lex_singles stream
   [< stream >] -> [< lex_singles stream >]

   Also there is nothing special about the words 'next_parser' or 'stream'
*)

let rec lex_singles = parser
                    | [< ' ((' ' | '\r' | '\n' | '\t'), loc); stream >] -> lex_singles stream

                    | [< ' (('A' .. 'Z' | 'a' .. 'z' as c), loc); stream >] ->
                      let buffer = Buffer.create 1 in
                      Buffer.add_char buffer c;
                      lex_ident buffer loc stream

                    | [< ' (('0' .. '9' as c), loc); stream >] ->
                      let buffer = Buffer.create 1 in
                      Buffer.add_char buffer c;
                      lex_number buffer loc stream

                    | [< ' ('/', loc); stream >] -> 
                      lex_comment_first (Token.char_to_token '/' loc) stream

                    | [< '(c, loc); stream >] ->
                      [< ' Token.char_to_token c loc; lex_singles stream >]

                    (*End of stream*)
                    | [< >] -> [< >]

and lex_ident buffer loc = parser
                         | [< ' ('A' .. 'Z' | 'a' .. 'z' | '0' .. '9' | '_' as c, _); stream>] -> (
                         Buffer.add_char buffer c;
                         lex_ident buffer loc stream )
                         | [< next_parser=lex_singles >] ->
                           [< 'Token.string_to_token (Buffer.contents buffer) loc; next_parser >]

and lex_number buffer loc = parser
                          | [< ' ('0' .. '9' as c, _); stream >] -> (
                            Buffer.add_char buffer c;
                            lex_number buffer loc stream )
                          | [< ' ('.', _); stream >] -> (
                            Buffer.add_char buffer '.';
                            lex_real_number buffer loc stream )
                          | [< next_parser=lex_singles >] ->
                            [< '(Token.Integer_Literal (int_of_string (Buffer.contents buffer)),
                             Span.span loc (Buffer.length buffer)); next_parser >]

and lex_real_number buffer loc = parser
                               | [< ' ('0' .. '9' as c, _); stream >] -> (
                                 Buffer.add_char buffer c;
                                 lex_real_number buffer loc stream )
                               | [< next_parser=lex_singles >] ->
                                 [< '(Token.Real_Literal (float_of_string (Buffer.contents buffer)),
                                      Span.span loc (Buffer.length buffer)); next_parser >]

and lex_comment_first tok = parser
                          | [< ' ('/', _); next_parser=lex_line_comment >] -> next_parser
                          | [< ' ('*', _); stream >] -> lex_multi_comment 1 stream
                          | [< next_parser=lex_singles >] -> [< ' tok; next_parser >]

and lex_line_comment = parser
                     | [< ' ('\n', _); next_parser=lex_singles >] -> next_parser
                     | [< '(c, _); next_parser=lex_line_comment >] -> next_parser
                     | [< >] -> [< >]

and lex_multi_comment level = parser
                            | [< ' ('*', _); stream >] -> lex_multi_comment_ending_first level stream
                            | [< ' ('/', _); stream >] -> lex_multi_comment_rec_first level stream
                            | [< '(c, _); stream >] -> lex_multi_comment level stream
                            | [< >] -> ignore(Log.log Log.Warning "Multiline comment never closed");
                                       [< >]

(* When we have seen a * we check for / *)
and lex_multi_comment_ending_first level = parser
                                         | [< ' ('/', _); stream >] ->
                                           if level = 1 then
                                             lex_singles stream
                                           else
                                             lex_multi_comment (level-1) stream
                                         | [< stream >] -> lex_multi_comment level stream

(* When we see a / in a multiline comment, we check for * in case of recursive comments *)
and lex_multi_comment_rec_first level = parser
                                      | [< ' ('*', _); stream >] -> lex_multi_comment (level+1) stream
                                      | [< stream >] -> lex_multi_comment level stream

let rec add_loc_to_char_stream line col =
  let loc : Span.loc_t = (line, col) in
  parser
| [< ' ('\n'); stream >] -> [< ' ('\n', loc); add_loc_to_char_stream (line+1) 0 stream >]
| [< ' ('\t'); stream >] -> [< ' ('\t', loc); add_loc_to_char_stream line (col+4) stream >]
| [< ' c; stream >] -> [< ' (c, loc); add_loc_to_char_stream line (col+1) stream >]
| [< >] -> [< >]

let lex char_stream = char_stream |> (add_loc_to_char_stream 1 0) |> lex_singles |> Token.merge_tokens_in_stream
