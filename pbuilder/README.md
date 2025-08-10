These helper scripts for pbuilder are used to construct a chroot system.
This chroot environments can then be used to build custom packages for openmediavault.

To install all dependencies, run the following command:
```shell
$ make install_deps
```

To create a new chroot system for a given platform, run the following command:
```shell
$ ./create.sh arm64
```

To build a package, run:
```shell
$ ./build.sh arm64 <PACKAGENAME>.dsc
```
