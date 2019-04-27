
let rec print_definitions defins =
  match defins with
  | defin :: rest -> print_endline (Daf_ast.string_of_definition 0 defin); print_definitions rest
  | [] -> ()

let () =
  match Util.at_array_opt Sys.argv 1 with
  | None ->
    Log.log Log.Fatal_Error "No input file";
  | Some input_file ->
    print_definitions (Parser.definition_list_of_file input_file)
