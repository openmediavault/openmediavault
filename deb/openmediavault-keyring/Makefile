build:
	mkdir -p trusted.gpg
	gpg --dearmor < keyrings/openmediavault-keyring.gpg > trusted.gpg/openmediavault-keyring.gpg
	gpg --dearmor < keyrings/openmediavault-archive-keyring.gpg > trusted.gpg/openmediavault-archive-keyring.gpg

clean:
	rm -rf trusted.gpg

install binary: build
	install -d $(DESTDIR)/usr/share/keyrings/
	cp keyrings/openmediavault-keyring.gpg $(DESTDIR)/usr/share/keyrings/
	cp keyrings/openmediavault-archive-keyring.gpg $(DESTDIR)/usr/share/keyrings/
	install -d $(DESTDIR)/etc/apt/trusted.gpg.d/
	cp $(shell find trusted.gpg/ -name '*.gpg' -type f) $(DESTDIR)/etc/apt/trusted.gpg.d/

.PHONY: clean build install binary
