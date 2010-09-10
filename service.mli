(** Windows service *)

exception Error of string

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
  val install : unit -> unit
  val remove : unit -> unit
  (** [run main]
    @param main function to run, stdin/stdout not available (will raise exception if used),
                when [!S.stop] is true this function should return as soon as possible *)
  val run : (unit -> unit) -> unit
end
