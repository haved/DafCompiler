
let scnd (_,x) = x

let at_array_opt array index = if Array.length array > index then Some array.(index) else None
