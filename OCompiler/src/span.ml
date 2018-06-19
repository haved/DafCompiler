type loc_t = {line:int; col:int}
type span_t = {loc:loc_t; len:int;}
type file_id_t = int
type snippet_t = span_t * file_id_t

let make (loc:loc_t) (len:int) :span_t = {loc=loc;len=len}

let loc span = span.loc
let end_loc span = {line=span.loc.line; col=span.loc.col+span.len}
let combine head tail = {loc=head.loc; len=tail.loc.col+tail.len-head.loc.col}
