FROM debian:buster
LABEL maintainer="volker.theile@openmediavault.org"

ENV container docker

ADD /shared/bin/ /bin
RUN /bin/omv-setup-docker.sh

VOLUME ["/openmediavault"]
VOLUME ["/shared"]

ENTRYPOINT ["/lib/systemd/systemd"]
