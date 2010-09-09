ocamlopt -verbose -c service_c.c 
ocamlopt -verbose -c -cclib -L/mingw/lib service.ml
ocamlmklib -verbose -L/mingw/lib service_c.o service.cmx -o service -oc service_c
ocamlopt -verbose service.cmxa unix.cmxa main.ml -o main.exe
