# Windows test environment

This Vagrant box can be used to test various features like SMB `Previos Versions` which are not available on Linux.

# Requirements

1. Install plugins
$ vagrant plugin install winrm

# Troubleshooting

- If you get the error `An error occurred executing a remote WinRM command.`,
  then make sure you are using Vagrant >= 2.3.8 
