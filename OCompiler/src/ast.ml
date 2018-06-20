
type infix_operator =
  | Plus | Minus | Mult | Divide

type expr_without_span =
  | Integer_Literal of int
  | Real_Literal of float
  | Infix_Operator of infix_operator * expr * expr

and expr = expr_without_span * Span.span_t

let print_definition defin =
  print_endline (match defin with
  | Plus -> "+"
  | Minus -> "-"
  | Mult -> "*"
  | Divide -> "/")
