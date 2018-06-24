
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
    print_definitions (Parser.file_to_definition_list input_file)
