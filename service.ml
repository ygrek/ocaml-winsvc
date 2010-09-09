
external install : unit -> unit = "caml_service_install"
external remove : unit -> unit = "caml_service_remove"
external run : (unit -> unit) -> (unit -> unit) -> unit = "caml_service_run"
external init : unit -> unit = "caml_service_init"

let run main stop = run main (fun () -> stop := true)

exception Error of string

let () =
  Callback.register_exception "Service.Error" (Error "register");
  init ()
