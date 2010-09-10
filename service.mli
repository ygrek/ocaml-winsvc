(** Windows service 

    Only one service in process is supported
*)

exception Error of string

(** Signature describing service hosted in this process *)
module type Sig =
sig
  (** Service identifier (unique) *)
  val name : string
  (** Service name displayed to user *)
  val display : string
  (** Service description *)
  val text : string
  (** Flag, signals that user requested service to stop *)
  val stop : bool ref
end

module Make(S : Sig) :
sig
  (** Install current executable as Windows service *)
  val install : unit -> unit
  (** Remove service *)
  val remove : unit -> unit
  (** [run main]
    @param main function to run, stdin/stdout not available (will raise exception if used),
                when [!S.stop] is true this function should return as soon as possible *)
  val run : (unit -> unit) -> unit
end
