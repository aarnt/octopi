
pkgname=octopi
pkgver=0.8.1
pkgrel=1
pkgdesc="This is Octopi, a powerful Pacman frontend using Qt libs"
url="https://octopiproject.wordpress.com/"
arch=('i686' 'x86_64')
license=('GPL2')
depends=('qt5-quickcontrols' 'pacman' 'pkgfile' 'knotifications' 'alpm_octopi_utils' 'xterm')
optdepends=('kdesu: for KDE'
            'gksu: for XFCE, Gnome, LXDE, Cinnamon'
            'gnome-keyring: for password management'
            'gist: for SysInfo report'
            'yaourt: for AUR support')
groups=('system')
install=octopi.install
source=("https://github.com/aarnt/octopi/archive/v${pkgver}.tar.gz")
md5sums=('669b6fa406ad64c65d9f548996cb3d8c')

prepare() {
   cd ${pkgname}-${pkgver}

   # enable the kstatus switch, disable if you wish to build without Plasma/knotifications support
   sed -e "s|DEFINES += ALPM_BACKEND #KSTATUS|DEFINES += ALPM_BACKEND KSTATUS|" -i notifier/octopi-notifier/octopi-notifier.pro
      
   cp resources/images/octopi_green.png resources/images/octopi.png
}
         
build() {
   cd ${pkgname}-${pkgver}
   
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
   cd ${pkgname}-${pkgver}
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
