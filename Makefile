VERSION=0.0.1
NAME=ocaml-winsvc-$(VERSION)

.PHONY: release
release:
	git tag -a -m $(VERSION) v$(VERSION)
	git archive --prefix=$(NAME)/ v$(VERSION) -o $(NAME).zip
	gpg -a -b $(NAME).zip
