open Printf

let binary_name = "dafc"
type log_level = Warning | Error | Fatal_Error

let string_of_log_level level = match level with
  | Warning -> "warning"
  | Error -> "error"
  | Fatal_Error -> "fatal_error"
let format_file_loc file_name (line, col) = (Printf.sprintf "%s: %d:%d" file_name line col)
let format_file_interval file_name ({loc=(line, col); end_loc=(end_line, end_col)}:Span.interval) =
  if line == end_line
    then Printf.sprintf "%s: %d:%d-%d" file_name line col end_col
    else Printf.sprintf "%s: %d:%d-%d:%d" file_name line col end_line end_col

let log_at at level text =
  Printf.printf "%s: %s: %s\n" at (string_of_log_level level) text;
  match level with
  | Fatal_Error -> ignore(exit 0)
  | _ -> ()

let log level text = log_at binary_name level text
let log_from_file file_name level text = log_at file_name text
let log_from_file_loc file_name loc level text = log_at (format_file_loc file_name loc) level text
let log_from_file_interval file_name interval level text = log_at (format_file_interval file_name interval) level text
let log_from_file_span file_name span level text = log_from_file_interval file_name (Span.interval_of_span span) level text

exception UnexpectedChar of char * Span.loc_t
exception UnexpectedToken of Token.token_with_span * string
exception UnexpectedEOF of string

let unexpected_token_msg token expected =
  (Printf.sprintf "unexpected token '%s'. Expected: %s" (Token.token_to_string token) expected)

let unexpected_EOF_msg expected = Printf.sprintf "expected %s, got EOF", expected
