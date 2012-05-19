
GEN=index.html
WEB=winsvc.forge.ocamlcore.org:/home/groups/winsvc/htdocs

html: $(GEN)

%.html: %.brtx
	brtx -html -doc -i $< -o $@

upload: index.html
	rsync -avhz $< $(WEB)/

clean:
	rm -f $(GEN)

