pkgname=octopi
pkgver=0.9.0
pkgrel=1
pkgdesc="This is Octopi, a powerful Pacman frontend using Qt libs"
url="https://octopiproject.wordpress.com/"
arch=('i686' 'x86_64')
license=('GPL2')
depends=('pacman' 'pacman-contrib' 'pkgfile' 'knotifications' 'alpm_octopi_utils' 'xterm' 'qtermwidget')
optdepends=('kdesu: for KDE'
            'gksu: for XFCE, Gnome, LXDE, Cinnamon'
            'gnome-keyring: for password management'
            'yaourt: for AUR support')
groups=('system')
install=octopi.install
source=("https://github.com/aarnt/octopi/archive/v${pkgver}.tar.gz")
#md5sums=('')

prepare() {
   cd ${pkgname}-${pkgver}

   # enable the kstatus switch, disable if you wish to build without Plasma/knotifications support
   sed -e "s|DEFINES += ALPM_BACKEND #KSTATUS|DEFINES += ALPM_BACKEND KSTATUS|" -i notifier/octopi-notifier/octopi-notifier.pro
      
   cp resources/images/octopi_green.png resources/images/octopi.png
}
         
build() {
   cd "${pkgname}-${pkgver}"
   
   qmake-qt5 PREFIX=/usr QMAKE_CFLAGS="${CFLAGS}" QMAKE_CXXFLAGS="${CXXFLAGS}" QMAKE_LFLAGS="${LDFLAGS}" octopi.pro
   make
   
   cd octopihelper
   qmake-qt5 PREFIX=/usr QMAKE_CFLAGS="${CFLAGS}" QMAKE_CXXFLAGS="${CXXFLAGS}" QMAKE_LFLAGS="${LDFLAGS}" octopi-helper.pro
   make
   cd ..
 
   cd notifier/octopi-notifier
   qmake-qt5 PREFIX=/usr QMAKE_CFLAGS="${CFLAGS}" QMAKE_CXXFLAGS="${CXXFLAGS}" QMAKE_LFLAGS="${LDFLAGS}" octopi-notifier.pro
   make
   cd ../..
   
   cd repoeditor
   qmake-qt5 PREFIX=/usr QMAKE_CFLAGS="${CFLAGS}" QMAKE_CXXFLAGS="${CXXFLAGS}" QMAKE_LFLAGS="${LDFLAGS}" octopi-repoeditor.pro
   make
   cd ..
   
   cd cachecleaner
   qmake-qt5 PREFIX=/usr QMAKE_CFLAGS="${CFLAGS}" QMAKE_CXXFLAGS="${CXXFLAGS}" QMAKE_LFLAGS="${LDFLAGS}" octopi-cachecleaner.pro
   make
}

package() {
   cd "${pkgname}-${pkgver}"
   make INSTALL_ROOT="${pkgdir}" install
   
   cd octopihelper
   make INSTALL_ROOT="${pkgdir}" install
   cd ..

   cd notifier/octopi-notifier
   make INSTALL_ROOT="${pkgdir}" install
   cd ../..
   
   cd repoeditor
   make INSTALL_ROOT="${pkgdir}" install
   cd ..
   
   cd cachecleaner
   make INSTALL_ROOT="${pkgdir}" install
}
