# Build Docker container
```
docker build --no-cache -t openmediavault-usul .
```

# Start the container
```
docker run -itd -v ~/git/openmediavault/docker/shared:/shared -v ~/git/openmediavault/deb:/openmediavault -v ~/.ssh:/root/.ssh:ro -v /dev:/dev -v /run/udev:/run/udev:ro -v /sys/fs/cgroup:/sys/fs/cgroup:ro --privileged --net=host --hostname=openmediavault-usul --add-host=openmediavault-usul:127.0.0.1 openmediavault-usul
```

# Log into the container
```
docker exec -it $(docker ps --no-trunc --quiet --filter "ancestor=openmediavault-usul") /bin/bash
```

# Install openmediavault
```
omv-install.sh
```

# Stop the container
```
docker stop $(docker ps --no-trunc --quiet --filter "ancestor=openmediavault-usul")
```
