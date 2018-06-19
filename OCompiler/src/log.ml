open Printf

let binary_name = "dafc"

let fatal_error text =
  Printf.printf "%s: FATAL_ERROR: %s\n" binary_name text;
  exit 0

let fatal_error_file file_name text =
  Printf.printf "%s: FATAL_ERROR: %s\n" file_name text;
  exit 0

let fatal_error_file_span file_name (span:Span.span_t) text =
  Printf.printf "%s: %d:%d: FATAL_ERROR: %s\n" file_name span.loc.line span.loc.col text;
  exit 0

exception SpanError of (Span.span_t * string)
exception ParseError of string

