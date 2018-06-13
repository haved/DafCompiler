open Printf

let rec print_token_stream stream =
    match Stream.peek stream with
      | None -> ()
      | Some token ->
        print_string (Token.token_to_string token);
        Stream.junk stream;
        print_token_stream stream

let tokenize file_name =
  let ic = open_in file_name in
  try
    let stream = Lexer.lex (Stream.of_channel ic) in
    stream
  with e ->
    close_in_noerr ic;
    raise e

let rec at_array index array = if Array.length array > index then Some array.(index) else None

let () =
  match at_array 1 Sys.argv with
  | None ->
    Log.fatal_error "No input file";
  | Some input_file ->
    print_token_stream (tokenize input_file)
