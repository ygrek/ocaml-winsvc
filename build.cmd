ocamlopt -verbose -c service_c.c
ocamlopt -verbose -c service.mli
ocamlopt -verbose -c -cclib -L/mingw/lib service.ml
ocamlc -verbose -c -cclib -L/mingw/lib service.ml
ocamlmklib -verbose -ocamlopt ocamlopt -L/mingw/lib service_c.o* service.cmx service.cmo -o service
ocamlopt -verbose service.cmxa unix.cmxa main.ml -o main.exe
