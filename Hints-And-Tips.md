This page contains various general hints and tips about RDQM.

# SELinux

If you have SELinux enabled and in enforcing mode, Ensure that you have set SELinux to treat DRBD in permissive mode by running the command `semanage permissive -a drbd_t` before creating an RDQM.

# rdqm.ini file

Make sure the rdqm.ini file is the same on all three nodes before you configure the HA Group.