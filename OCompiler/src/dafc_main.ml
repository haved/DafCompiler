
let get_definitions file_name =
  let ic = open_in file_name in
  try
    let stream = Lexer.lex (Stream.of_channel ic) in
    let defins = Parser.parse_all_definitions [] stream in
    close_in ic;
    defins
  with
  | Log.UnexpectedToken ((token, span), expected) as e ->
    Log.log_from_file_span file_name span Log.Fatal_Error (Log.unexpected_token_msg token expected);
    raise e
  | e ->
    close_in_noerr ic;
    raise e

let rec print_definitions defins =
  match defins with
  | defin :: rest -> Ast.print_definition defin; print_definitions rest
  | [] -> ()

let rec at_array index array = if Array.length array > index then Some array.(index) else None

let () =
  match at_array 1 Sys.argv with
  | None ->
    Log.log Log.Fatal_Error "No input file";
  | Some input_file ->
    print_definitions (get_definitions input_file)
