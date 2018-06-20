open Printf

let rec print_token_stream stream =
    match Stream.peek stream with
      | None -> ()
      | Some (token, _) ->
        Printf.printf "%s " (Token.token_to_string token);
        Stream.junk stream;
        print_token_stream stream

let compile file_name =
  let ic = open_in file_name in
  try
    let stream = Lexer.lex (Stream.of_channel ic) in
    print_token_stream stream;
  with
  | Stream.Failure -> close_in ic; (*File is done*)
  | Log.UnexpectedToken (tok_with_span, expected) -> Log.unexpected_token_msg file_name tok_with_span expected
  | e ->
    close_in_noerr ic;
    raise e

let rec at_array index array = if Array.length array > index then Some array.(index) else None

let () =
  match at_array 1 Sys.argv with
  | None ->
    Log.log Log.Fatal_Error "No input file";
  | Some input_file ->
    compile input_file
