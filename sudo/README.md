## OCTOPI-SUDO

This is a clone of LXQt sudo tool (without LXQt libs). It is the *ONLY* priviledge escalation tool supported by Octopi.

### Using Octopi without password

From version 0.10.0 on, Octopi uses a new priviledge escalation tool based on [LXQt sudo project](https://github.com/lxqt/lxqt-sudo).
To use Octopi without a password your user need to be member of "wheel" group and you must run the
following command as root (just one time):

```
$ \usr\lib\octopi\octopi-sudo -setnopasswd
```
