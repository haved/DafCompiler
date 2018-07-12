type loc_t = int * int
type span_t = {loc:loc_t; len:int}
let span loc len = {loc=loc; len=len}

let end_loc {loc=(line, col); len=len} = (line, col+len)
let combine_spans_opt span1 {loc=loc2; len=len2} =
  if end_loc span1 = loc2  then Some {loc=span1.loc; len=span1.len+len2} else None

type interval_t = {loc: loc_t; end_loc: loc_t}
let interval_of_span (span:span_t) = {loc=span.loc; end_loc=end_loc span}
let interval_of_spans {loc=start_loc} span2 = {loc=start_loc; end_loc=end_loc span2}
let interval_merge {loc=start1} {end_loc=stop2} : interval_t = {loc=start1; end_loc=stop2}
