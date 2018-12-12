
let first_some list final =
  match list with
  | [] -> final
  | head :: rest -> match head with
                    | Some x -> x
                    | None -> first_some rest final

let scnd (_,x) = x

let thrd (_,_,x) = x

let at_array_opt array index = if Array.length array > index then Some array.(index) else None
