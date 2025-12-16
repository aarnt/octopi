## This is Octopi, a powerful Pacman/AUR front end using Qt libs.

![Main window](https://raw.githubusercontent.com/aarnt/octopi/master/octopi-mainwindow.png)
![Options dialog](https://raw.githubusercontent.com/aarnt/octopi/master/octopi-optionsdialog.png)
![Main window with qss](https://raw.githubusercontent.com/aarnt/octopi/master/octopi-mainwindow-with-qss.png)

The project site is hosted on https://tintaescura.com/projects/octopi

Currently, 11 Linux distros are compatible with it

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

### What you must install in order to have Octopi fully functional

You'll need:
 * [Alpm_octopi_utils](https://github.com/aarnt/alpm_octopi_utils/) library
 * A helper to execute pacman commands called "octphelper", available on "./helper" dir
 * A privilege escalation tool called [qt-sudo](https://github.com/aarnt/qt-sudo/) (at least version 2.3.0 if you run a Plasma session)
 * qtermwidget package, in order to build Octopi with embedded terminal support

### To install Octopi using pacman

If Octopi package is available in your distro's repository, you can just type:

```
# pacman -S octopi
```

### Steps to build Octopi source code (qmake)

Assuming you have vala compiler and Qt6 libs properly installed, follow these steps:

```
$ git clone https://github.com/aarnt/alpm_octopi_utils
$ cd alpm_octopi_utils
$ make
# make install
$ cd ..
$ git clone https://github.com/aarnt/qt-sudo
$ cd qt-sudo
$ qmake6
$ make
# make install
$ cd ..
$ git clone https://github.com/aarnt/octopi
$ cd octopi/helper
$ qmake6
$ make
# make install
$ cd ../notifier
$ qmake6
$ make
# make install
$ cd ../cachecleaner
$ qmake6
$ make
# make install
$ cd ../repoeditor
$ qmake6
$ make
# make install
$ cd ..
$ qmake6
$ make
# make install
```

You can also use the available PKGBUILD script that helps you build latest Octopi development version with all its tools(*):

```
$ cd OCTOPI_PATH (where you git cloned the source code)
$ makepkg -f
```

(*) It may contain bugs. You have been warned.

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
In KaOS, [kcp](https://codeberg.org/bvaudour/kcp) will be supported out of the box.

### Ways to help/support Octopi

 * You can "Star" it on the Github page - https://github.com/aarnt/octopi/
 * You can vote in the AUR package available on https://aur.archlinux.org/packages/octopi/
 * You can translate it to your mother language on https://explore.transifex.com/arnt/octopi/
 * You can follow author's twitter account on https://twitter.com/aaarnt
 * You can buy author's technical book (currently in portuguese) about Octopi and Qt5 on
http://www.amazon.com.br/Aprendendo-Qt-com-projeto-Octopi-ebook/dp/B015ICHKV6
 * You can buy author's poem book (currently in portuguese) on meditation, Buddhism, cosmology and other subjects on
https://www.amazon.com.br/Avidya-Alexandre-Arnt-ebook/dp/B0965LVWR3
 * You can write a review about it (text / video)
 * You can donate money to the author's Paypal - http://sourceforge.net/donate/index.php?group_id=186459
 * You can join the project ;-)


Enjoy!
