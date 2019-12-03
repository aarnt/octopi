## This is Octopi, a powerful Pacman/AUR frontend using Qt libs.

![Main window](https://raw.githubusercontent.com/aarnt/octopi/master/octopi-mainwindow.png)

The project site is hosted at https://octopiproject.wordpress.com

Currently, 10 Linux distros are compatible with it

 * [Antergos](https://antergos.com/)
 * [ArchBang](http://archbang.org/)
 * [Arch](https://www.archlinux.org/)
 * [Chakra](https://chakralinux.org/)
 * [CondresOS](https://condresos.codelinsoft.it/)
 * [EndeavourOS](https://endeavouros.com/)
 * [KaOS](https://kaosx.us/)
 * [Manjaro](https://manjaro.org/)
 * [Netrunner (rolling)](http://www.netrunner.com/netrunner-rolling-release/)
 * [Parabola GNU/Linux-libre](https://www.parabola.nu/)

### What you must install in order to have Octopi fully functional

You'll need:
 * [Alpm_octopi_utils](https://github.com/aarnt/alpm_octopi_utils/) library
 * A helper to execute pacman commands called "octphelper", available at "./helper" dir
 * A priviledge escalation tool called "octopi-sudo", available at "./sudo" dir
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

### How to enable AUR support (that "alien" icon at toolbar)

To enable AUR support, you'll need to install [yaourt](https://archlinux.fr/yaourt-en), 
[pacaur](https://github.com/rmarquis/pacaur), [pikaur](https://github.com/actionless/pikaur), 
[trizen](https://github.com/trizen/trizen) or [yay](https://github.com/Jguer/yay) in your system. 
If neither of the previous tools are found Octopi will download latest "yay-bin" github binary.
In Chakra, [chaser](https://github.com/ccr-tools/chaser) will be supported out of the box.
In KaOS, [kcp](https://github.com/bvaudour/kcp) will be supported out of the box.

### Ways to help/support Octopi

 * You can "Star" it at the Github page - https://github.com/aarnt/octopi/
 * You can vote in the AUR package available at https://aur.archlinux.org/packages/octopi/
 * You can translate it to your mother language at https://www.transifex.com/projects/p/octopi/
 * You can follow author's twitter account at https://twitter.com/aaarnt
 * You can buy author's technical book (currently in portuguese) about Octopi and Qt5 at 
http://www.amazon.com.br/Aprendendo-Qt-com-projeto-Octopi-ebook/dp/B015ICHKV6
 * You can write a review about it (text / video)
 * You can donate money to the project Paypal - http://sourceforge.net/donate/index.php?group_id=186459
 * You can join the project ;-)


Enjoy!
