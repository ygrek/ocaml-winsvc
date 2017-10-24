
external install : string -> string -> string -> string -> unit = "caml_service_install"
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
  val arguments : string list
  val stop : unit -> unit
end

module Make(S : Sig) =
struct
  let args = 
    List.map Filename.quote S.arguments
  let path =
    String.concat " " (Sys.executable_name :: args)
  let install () =
    install S.name S.display S.text path
  let remove () = remove S.name
  let run main = run S.name main S.stop
end
