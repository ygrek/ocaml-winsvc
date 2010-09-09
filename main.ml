
let f = open_out "\\service.log"
let pr = Printf.fprintf f "%s\n%!"

let should_stop = ref false
let stop () = should_stop := true
let run () = 
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

let () = Callback.register "service.run" run
let () = Callback.register "service.stop" stop
