type loc_t = int * int
type span_t = loc_t * loc_t

type end_loc_t = loc_t

let loc_add (line,col) len :end_loc_t = (line,col+len)
let span_of_loc_len loc len = (loc, loc_add loc len)

let span_over (start,_) (_,end_loc) = (start,end_loc)
let span start_loc end_loc = (start_loc, end_loc)
let combine_spans_opt (s1,e1) (s2,e2) = if e1 = s2 then Some (s1,e2) else None
