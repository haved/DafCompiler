open Printf

let binary_name = "dafc"

let fatal_error text =
  Printf.printf "%s: FATAL_ERROR: %s\n" binary_name text;
  exit 0
