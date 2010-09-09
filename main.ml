
let f = open_out "\\service.log"
let pr = Printf.fprintf f "%s\n%!"

let should_stop = ref false
let run () = 
  Gc.compact ();
  let rec wait x =
    if !should_stop then () else
    begin
      Unix.sleep 1;
      wait (x+1)
    end 
  in
  pr "running";
  wait 0;
  pr "finished";
  close_out f

let () =
  match List.tl (Array.to_list Sys.argv) with
  | ["-install"] -> Service.install ()
  | ["-remove"] -> Service.remove ()
  | [] -> Service.run run should_stop
  | _ -> print_endline "doing nothing"
