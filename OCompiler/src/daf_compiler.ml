open Printf

let input_file = "TestFile.daf"

type token =
  | Def

let tokenize file_name =
  let ic = open_in file_name in
  try
    close_in ic;
    file_name
  with e ->
    close_in_noerr ic;
    raise e

(*let () =*)

