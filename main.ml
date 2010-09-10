
let f = open_out "\\service.log"
let pr = Printf.fprintf f "%s\n%!"

module S = struct
let name = "test"
let display = "Test service"
let text = "Test service written in OCaml"
let stop = ref false
end

module Svc = Service.Make(S)

let main () = 
  pr "main";
  Gc.compact ();
  pr "running";
  while not !S.stop do Unix.sleep 1 done;
  pr "finished"

let () =
  match List.tl (Array.to_list Sys.argv) with
  | ["-install"] -> Svc.install ()
  | ["-remove"] -> Svc.remove ()
  | [] -> pr "run"; (try Svc.run main with exn -> pr (Printexc.to_string exn))
  | _ -> print_endline "doing nothing"

let () =
  close_out f