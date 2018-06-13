open Printf

let rec print_token_stream stream =
    match Stream.peek stream with
      | None -> ()
      | Some token ->
        print_string (Token.token_to_string token);
        try
          Stream.junk stream;
          print_token_stream stream
        with e -> ()

let compile file_name =
  let ic = open_in file_name in
  try
    let stream = Lexer.lex (Stream.of_channel ic) in
    print_token_stream stream;
    close_in ic;
  with e ->
    close_in_noerr ic;
    raise e

let rec at_array index array = if Array.length array > index then Some array.(index) else None

let () =
  match at_array 1 Sys.argv with
  | None ->
    Log.fatal_error "No input file";
  | Some input_file ->
    compile input_file
