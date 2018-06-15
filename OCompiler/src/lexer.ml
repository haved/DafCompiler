
(* These are identical:
   [< next_parser = lex_singles >] -> next_parser
   [< next_parser = lex_singles >] -> [< next_parser >]
   [< stream >] -> lex_singles stream
   [< stream >] -> [< lex_singles stream >]

   Also there is nothing special about the words 'next_parser' or 'stream'
*)

(* turns a char stream into a token stream, but makes tokens as short as possible
   Token.merge_tokens_in_stream combines tokens
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
              lex_comment_first stream

            | [< '(c, loc); stream >] ->
              [< ' Token.char_to_token c loc; lex_singles stream >]

            (*End of stream*)
            | [< >] -> [< >]

and lex_ident buffer loc = parser
                     | [< ' ('A' .. 'Z' | 'a' .. 'z' | '0' .. '9' | '_' as c); stream>] ->
                       Buffer.add_char buffer c;
                       lex_ident buffer loc stream
                     | [< next_parser=lex_singles >] -> [< 'Token.string_to_token (Buffer.contents buffer) loc; next_parser >]

and lex_number buffer loc = parser
                      | [< ' ('0' .. '9' as c); stream >] ->
                        Buffer.add_char buffer c;
                        lex_number buffer loc stream
                      | [< ' ('.'); stream >] ->
                        Buffer.add_char buffer '.';
                        lex_real_number buffer loc stream
                      | [< next_parser=lex_singles >] ->
                        [< '(Token.Integer_Literal (int_of_string (Buffer.contents buffer)), (loc, Buffer.length buffer)); next_parser >]

and lex_number buffer loc = parser
                      | [< ' ('0' .. '9' as c); stream >] ->
                        Buffer.add_char buffer c;
                        lex_number buffer loc stream
                      | [< next_parser=lex_singles >] ->
                        [< '(Token.Real_Literal (float_of_string (Buffer.contents buffer)), (loc, Buffer.length buffer)); next_parser >]

and lex_comment_first = parser
                      | [< ' ('/'); next_parser=lex_line_comment >] -> next_parser
                      | [< ' ('*'); next_parser=lex_multi_comment >] -> next_parser
                      | [< next_parser=lex_singles >] -> [< ' (Token.char_to_token '/'); next_parser >]

and lex_line_comment = parser
                     | [< ' ('\n'); next_parser=lex_singles >] -> next_parser
                     | [< 'c; next_parser=lex_line_comment >] -> next_parser
                     | [< >] -> [< >]

and lex_multi_comment = parser
                      | [< ' ('*'); next_parser=lex_multi_comment_ending_first >] -> next_parser
                      | [< 'c; next_parser=lex_multi_comment >] -> next_parser
                      | [< >] -> [< >]

and lex_multi_comment_ending_first = parser
                                   | [< ' ('/'); next_parser=lex_singles >] -> next_parser
                                   | [< next_parser=lex_multi_comment >] -> next_parser

let rec add_loc_to_char_stream line col = parser
                                    | [< ' ('\n'); next=(add_loc_to_char_stream (line+1) 0) >] -> [< ' ('\n', (line, col)); next >]
                                    | [< ' ('\t'); next=(add_loc_to_char_stream line (col+4)) >] -> [< ' ('\t', (line, col)); next >]
                                    | [< ' c; next=(add_loc_to_char_stream line (col+4)) >] -> [< ' (c, (line, col)); next >]
                                    | [< >] -> [< >]

let lex char_stream = char_stream |> (add_loc_to_char_stream 1 0) |> lex_singles |> Token.merge_tokens_in_stream
