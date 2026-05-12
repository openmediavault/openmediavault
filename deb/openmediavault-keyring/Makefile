build:
	mkdir -p keyring-gpg
	gpg --dearmor < keyring-asc/openmediavault-keyring.asc > keyring-gpg/openmediavault-keyring.gpg
	gpg --dearmor < keyring-asc/openmediavault-archive-keyring.asc > keyring-gpg/openmediavault-archive-keyring.gpg

clean:
	rm -rf keyring-gpg

install binary: build
	install -d $(DESTDIR)/usr/share/keyrings/
	cp $(shell find keyring-gpg/ -name '*.gpg' -type f) $(DESTDIR)/usr/share/keyrings/
	install -d $(DESTDIR)/etc/apt/trusted.gpg.d/
	cp $(shell find keyring-gpg/ -name '*.gpg' -type f) $(DESTDIR)/etc/apt/trusted.gpg.d/

.PHONY: clean build install binary
