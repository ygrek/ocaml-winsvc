
open Printf

let f = lazy (open_out_gen [Open_text;Open_creat;Open_append;Open_wronly] 0o640 "\\service.log")
let pr = fprintf (Lazy.force f) "%s\n%!"

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
  | ["-install"] -> Svc.install (); printf "Installed %S" S.name
  | ["-remove"] -> Svc.remove (); printf "Removed %S" S.name
  | [] -> 
      (* launched as service by SCM *)
      begin try Svc.run main with exn -> pr (Printexc.to_string exn) end
  | _ -> print_endline "doing nothing"
