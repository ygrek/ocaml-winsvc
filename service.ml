
external install : string -> string -> string -> unit = "caml_service_install"
external remove : string -> unit = "caml_service_remove"
external run : string -> (unit -> unit) -> (unit -> unit) -> unit = "caml_service_run"
external init : unit -> unit = "caml_service_init"

exception Error of string

let () =
  Callback.register_exception "Service.Error" (Error "register");
  init ()

module type Sig =
sig
  val name : string
  val display : string
  val text : string
  val stop : bool ref
end

module Make(S : Sig) =
struct
  let install () = install S.name S.display S.text
  let remove () = remove S.name
  let run main = run S.name main (fun () -> S.stop := true)
end
