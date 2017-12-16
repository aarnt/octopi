
pkgname=octopi
pkgver=0.8.1
pkgrel=1
pkgdesc="This is Octopi, a powerful Pacman frontend using Qt libs"
url="https://octopiproject.wordpress.com/"
arch=('i686' 'x86_64')
license=('GPL2')
depends=('pacman' 'pkgfile' 'knotifications' 'alpm_octopi_utils' 'xterm' 'qtermwidget')
optdepends=('kdesu: for KDE'
            'gksu: for XFCE, Gnome, LXDE, Cinnamon'
            'gnome-keyring: for password management'
            'gist: for SysInfo report'
            'yaourt: for AUR support'
            'pacaur: for AUR support'
            'pacmanlogviewer: to view pacman log files')
groups=('system')
install=octopi.install
source=("git+https://github.com/aarnt/octopi.git")
md5sums=('SKIP')

pkgver() {
   cd ${pkgname}
   printf $_pkgver".r%s.%s" "$(git rev-list --count HEAD)" "$(git rev-parse --short HEAD)"
}

prepare() {
   cd ${pkgname}

   # enable the kstatus switch, disable if you wish to build without Plasma/knotifications support
   sed -e "s|DEFINES += ALPM_BACKEND QTERMWIDGET #KSTATUS|DEFINES += ALPM_BACKEND QTERMWIDGET KSTATUS|" -i notifier/octopi-notifier/octopi-notifier.pro
      
   cp resources/images/octopi_green.png resources/images/octopi.png
}
         
build() {
   cd ${pkgname}
   
   qmake-qt5 octopi.pro
   make
   
   cd notifier/pacmanhelper
   qmake-qt5 pacmanhelper.pro
   make
   cd ../..
   
   cd notifier/octopi-notifier
   qmake-qt5 octopi-notifier.pro
   make
   cd ../..
   
   cd repoeditor
   qmake-qt5 octopi-repoeditor.pro
   make
   cd ..
   
   cd cachecleaner
   qmake-qt5 octopi-cachecleaner.pro
   make
}

package() {
   cd ${pkgname}
   make INSTALL_ROOT=${pkgdir} install
   
   cd notifier/pacmanhelper
   make INSTALL_ROOT=${pkgdir} install
   cd ../..
   
   cd notifier/octopi-notifier
   make INSTALL_ROOT=${pkgdir} install
   cd ../..
   
   cd repoeditor
   make INSTALL_ROOT=${pkgdir} install
   cd ..
   
   cd cachecleaner
   make INSTALL_ROOT=${pkgdir} install
}
