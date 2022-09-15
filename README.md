## This is Octopi, a powerful Pacman/AUR front end using Qt libs.

![Main window](https://raw.githubusercontent.com/aarnt/octopi/master/octopi-mainwindow.png)

The project site is hosted on https://tintaescura.com/projects/octopi

Currently, 12 Linux distros are compatible with it

 * [ArchBang](http://archbang.org/)
 * [Archcraft](https://archcraft.io/)
 * [Arch Linux](https://www.archlinux.org/)
 * [ArcoLinux](https://arcolinux.info/)
 * [Artix Linux](https://artixlinux.org)
 * [CachyOS](https://cachyos.org/)
 * [EndeavourOS](https://endeavouros.com/)
 * [Garuda Linux](https://garudalinux.org/)
 * [KaOS](https://kaosx.us/)
 * [Manjaro](https://manjaro.org/)
 * [Obarun Linux](https://web.obarun.org/index.php?id=1)
 * [Parabola GNU/Linux-libre](https://www.parabola.nu/)

### What you must install in order to have Octopi fully functional

You'll need:
 * [Alpm_octopi_utils](https://github.com/aarnt/alpm_octopi_utils/) library
 * A helper to execute pacman commands called "octphelper", available on "./helper" dir
 * A privilege escalation tool called "octopi-sudo", available on "./sudo" dir
 * qtermwidget package, in order to build Octopi with embedded terminal support

### To install Octopi using pacman

If Octopi package is available in your distro's repository, you can just type:

```
# pacman -S octopi
```

### Steps to build Octopi source code (qmake)

Assuming you have vala compiler and Qt5 libs properly installed, follow these steps:

```
$ git clone https://github.com/aarnt/alpm_octopi_utils
$ cd alpm_octopi_utils
$ make
# make install
$ cd ..
$ git clone https://github.com/aarnt/octopi
$ cd octopi/sudo
$ qmake-qt5
$ make
# make install
$ cd ../helper
$ qmake-qt5
$ make
# make install
$ cd ../notifier
$ qmake-qt5
$ make
# make install
$ cd ../cachecleaner
$ qmake-qt5
$ make
# make install
$ cd ../repoeditor
$ qmake-qt5
$ make
# make install
$ cd ..
$ qmake-qt5
$ make
# make install
```

You can also use the available PKGBUILD script that helps you build Octopi with all its tools:

```
$ cd OCTOPI_PATH (where you git cloned the source code)
$ makepkg -f
```

### Steps to build Octopi source code (CMake)

As an alternative to qmake, Octopi can also be built with CMake. Make sure that at least CMake 3.5 is installed.

First, build and install alpm_octopi_utils:

```
$ git clone https://github.com/aarnt/alpm_octopi_utils
$ cd alpm_octopi_utils
$ mkdir build_dir && cd build_dir
$ cmake -G "Unix Makefiles" .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
$ make
$ sudo make install
```

Next, build and install Octopi:

```
$ git clone https://github.com/aarnt/octopi
$ cd octopi
$ mkdir build_dir && cd build_dir
$ cmake -G "Unix Makefiles" .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
$ make
$ sudo make install
```

### To run Octopi

```
$ /usr/bin/octopi
```

### To run Octopi Notifier

```
$ /usr/bin/octopi-notifier
```

### To enable AUR support (that "green alien" icon on toolbar)

You'll need to install [pacaur](https://github.com/rmarquis/pacaur), [paru](https://github.com/morganamilo/paru),
[pikaur](https://github.com/actionless/pikaur), [trizen](https://github.com/trizen/trizen) or
[yay](https://github.com/Jguer/yay) in your system.
If neither of the previous tools are found Octopi will download latest "yay-bin" github binary.
In Chakra, [chaser](https://github.com/ccr-tools/chaser) will be supported out of the box.
In KaOS, [kcp](https://github.com/bvaudour/kcp) will be supported out of the box.

### Ways to help/support Octopi

 * You can "Star" it on the Github page - https://github.com/aarnt/octopi/
 * You can vote in the AUR package available on https://aur.archlinux.org/packages/octopi/
 * You can translate it to your mother language on https://www.transifex.com/projects/p/octopi/
 * You can follow author's twitter account on https://twitter.com/aaarnt
 * You can buy author's technical book (currently in portuguese) about Octopi and Qt5 on
http://www.amazon.com.br/Aprendendo-Qt-com-projeto-Octopi-ebook/dp/B015ICHKV6
 * You can buy author's poem book (currently in portuguese) on meditation, Buddhism, cosmology and other subjects on
https://www.amazon.com.br/Avidya-Alexandre-Arnt-ebook/dp/B0965LVWR3
 * You can write a review about it (text / video)
 * You can donate money to the author's Paypal - http://sourceforge.net/donate/index.php?group_id=186459
 * You can join the project ;-)


Enjoy!
