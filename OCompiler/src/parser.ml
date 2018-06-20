
let rec parseExpression = parser
                        | [< ' (Token.Integer_Literal num, span) >] -> (Ast.Integer_Literal num,span)
                        | [< ' token_with_span >] -> raise (Log.UnexpectedToken (token_with_span, "expression"))
                        | [< >] -> raise (Log.UnexpectedEOF "expression")

and parse_definition = parser
                     | [< ' (Token.Def, loc) >] -> Ast.Minus
                     | [< ' token_with_span >] -> raise (Log.UnexpectedToken (token_with_span, "definition"))
                     | [< >] -> raise (Log.UnexpectedEOF "definition")

and parse_all_definitions list = parser
                               | [< >] -> list
                               | [< stream >] -> parse_all_definitions [list :: parse_definition stream] stream
