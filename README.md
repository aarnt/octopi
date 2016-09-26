## This is Octopi, a powerful Pacman/AUR frontend using Qt libs.

The project site is hosted at https://octopiproject.wordpress.com

Currently, 10 Linux distros are compatible with it

 * [Antergos] (https://antergos.com/)
 * [ArchBang] (http://wiki.archbang.org/index.php?title=Main_Page)
 * [Arch] (https://www.archlinux.org/)
 * [ArchBSD] (https://pacbsd.org/)
 * [Chakra] (https://chakralinux.org/)
 * [KaOS] (http://kaosx.us/)
 * [Manjaro] (http://manjaro.github.io/)
 * [mooOS] (http://mooos.org/)
 * [Netrunner (rolling)] 
(http://www.netrunner.com/netrunner-rolling-release/)
 * [Parabola GNU/Linux-libre] (http://www.parabola.nu/https/)

### What you must install in your system to have Octopi fully functional

You'll need:
 * [Alpm_octopi_utils] (https://github.com/aarnt/alpm_octopi_utils/) library
 * A privilege escalation tool to use it, once running as root will ONLY work in KDE. Octopi supports 
gksu, kdesu and lxqt-sudo for that
 
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

### How to enable the "alien" icon at toolbar

To enable AUR support, you'll need to install "yaourt" or "pacaur" in your system.  
In Chakra, "chaser" will be supported out of the box.  
In KaOS, "kcp" will be supported out of the box.

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
