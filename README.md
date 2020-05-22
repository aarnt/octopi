## This is Octopi, a powerful Pacman/AUR frontend using Qt libs.

![Main window](https://raw.githubusercontent.com/aarnt/octopi/master/octopi-mainwindow.png)

The project site is hosted on https://tintaescura.com/projects/octopi

Currently, 9 Linux distros are compatible with it

 * [ArchBang](http://archbang.org/)
 * [Arch Linux](https://www.archlinux.org/)
 * [ArcoLinux](https://arcolinux.info/)
 * [Chakra](https://chakralinux.org/)
 * [CondresOS](https://condresos.codelinsoft.it/)
 * [EndeavourOS](https://endeavouros.com/)
 * [KaOS](https://kaosx.us/)
 * [Manjaro](https://manjaro.org/)
 * [Parabola GNU/Linux-libre](https://www.parabola.nu/)

### What you must install in order to have Octopi fully functional

You'll need:
 * [Alpm_octopi_utils](https://github.com/aarnt/alpm_octopi_utils/) library
 * A helper to execute pacman commands called "octphelper", available on "./helper" dir
 * A priviledge escalation tool called "octopi-sudo", available on "./sudo" dir
 * qtermwidget >= 0.14.1 in order to build Octopi with embedded terminal support
 
### Simple steps to build Octopi code

Assuming you have Qt5 libs properly installed, go to the directory where the code is located:

```
$ cd OCTOPI_PATH
$ qmake-qt5
$ make
```

You can also use the available PKGBUILD script that helps you build Octopi with all its tools:

```
$ cd OCTOPI_PATH
$ makepkg -f
```

### How to enable AUR support (that "alien" icon on toolbar)

To enable AUR support, you'll need to install [pacaur](https://github.com/rmarquis/pacaur), 
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
 * You can write a review about it (text / video)
 * You can donate money to the project Paypal - http://sourceforge.net/donate/index.php?group_id=186459
 * You can join the project ;-)


Enjoy!
