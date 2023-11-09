# Using VirtualBox

1. Install
```
$ sudo apt-get install virtualbox
```

2. Install NFS kernel server
```
$ sudo apt-get install nfs-kernel-server
```

3. Download Vagrant from https://www.vagrantup.com/downloads.html
```
$ sudo apt-get install vagrant
```

# Using libvirt

1. Install
```
$ sudo apt install qemu-kvm virt-manager
```

2. Install NFS kernel server
```
$ sudo apt-get install nfs-kernel-server
```

3. Download Vagrant from https://www.vagrantup.com/downloads.html
```
$ sudo apt-get install vagrant vagrant-libvirt
```

4. Install vagrant_reboot_linux plugin (https://github.com/secret104278/vagrant_reboot_linux)
$ vagrant plugin install vagrant_reboot_linux

# Troubleshooting

- If you get the error `mount.nfs: access denied by server while mounting ...`,
  then make sure your home directory `/home/<USER>/` is accessible (`r-x`)
  by `others`.
