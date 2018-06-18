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
    try
      print_token_stream stream;
    with
    | Stream.Failure -> close_in ic; (*File is done*)
    | e -> raise e;
  with file_error ->
    close_in_noerr ic;
    raise file_error

let rec at_array index array = if Array.length array > index then Some array.(index) else None

let () =
  match at_array 1 Sys.argv with
  | None ->
    Log.fatal_error "No input file";
  | Some input_file ->
    compile input_file
