type loc_t = int * int
type span_t = loc_t * loc_t

let end_loc (line,col) len = (line,col+len)
let span_of_loc_len loc len = (loc, end_loc loc len)

let span_over (start,_) (_,end_loc) = (start,end_loc)
let combine_spans_opt (s1,e1) (s2,e2) = if e1 == s2 then Some (s1,e2) else None
