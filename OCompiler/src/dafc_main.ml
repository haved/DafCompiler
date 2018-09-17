
let rec print_definitions defins =
  match defins with
  | defin :: rest -> print_endline (Daf_ast.string_of_definition defin); print_definitions rest
  | [] -> ()

let rec at_array_opt index array = if Array.length array > index then Some array.(index) else None

let () =
  match at_array_opt 1 Sys.argv with
  | None ->
    Log.log Log.Fatal_Error "No input file";
  | Some input_file ->
    print_definitions (Parser.definition_list_of_file input_file)
