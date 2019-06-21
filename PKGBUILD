pkgname=octopi
pkgver=0.10.0
pkgrel=1
pkgdesc="This is Octopi, a powerful Pacman frontend using Qt libs"
url="https://octopiproject.wordpress.com/"
arch=('i686' 'x86_64')
license=('GPL2')
depends=('alpm_octopi_utils' 'pacman' 'pacman-contrib' 'pkgfile' 'qtermwidget' 'sudo')
groups=('system')
install=octopi.install
source=("https://github.com/aarnt/octopi/archive/master.zip")
md5sums=('SKIP')

prepare() {
   cd "${pkgname}"-master
   
   # enable the kstatus switch, disable if you wish to build without Plasma/knotifications support
   sed -e "s|DEFINES += ALPM_BACKEND #KSTATUS|DEFINES += ALPM_BACKEND KSTATUS|" -i notifier/octopi-notifier.pro
      
   cp resources/images/octopi_green.png resources/images/octopi.png
}
         
build() {
   cd "${pkgname}"-master
   
   qmake-qt5 PREFIX=/usr QMAKE_CFLAGS="${CFLAGS}" QMAKE_CXXFLAGS="${CXXFLAGS}" QMAKE_LFLAGS="${LDFLAGS}" octopi.pro
   make
   
   cd helper
   qmake-qt5 PREFIX=/usr QMAKE_CFLAGS="${CFLAGS}" QMAKE_CXXFLAGS="${CXXFLAGS}" QMAKE_LFLAGS="${LDFLAGS}" octopi-helper.pro
   make
   cd ..
 
   cd notifier
   qmake-qt5 PREFIX=/usr QMAKE_CFLAGS="${CFLAGS}" QMAKE_CXXFLAGS="${CXXFLAGS}" QMAKE_LFLAGS="${LDFLAGS}" octopi-notifier.pro
   make
   cd ..
   
   cd repoeditor
   qmake-qt5 PREFIX=/usr QMAKE_CFLAGS="${CFLAGS}" QMAKE_CXXFLAGS="${CXXFLAGS}" QMAKE_LFLAGS="${LDFLAGS}" octopi-repoeditor.pro
   make
   cd ..
   
   cd cachecleaner
   qmake-qt5 PREFIX=/usr QMAKE_CFLAGS="${CFLAGS}" QMAKE_CXXFLAGS="${CXXFLAGS}" QMAKE_LFLAGS="${LDFLAGS}" octopi-cachecleaner.pro
   make
   cd ..

   cd sudo
   qmake-qt5 PREFIX=/usr QMAKE_CFLAGS="${CFLAGS}" QMAKE_CXXFLAGS="${CXXFLAGS}" QMAKE_LFLAGS="${LDFLAGS}" octopi-sudo.pro
   make
}

package() {
   cd "${pkgname}"-master
   make INSTALL_ROOT="${pkgdir}" install
   
   cd helper
   make INSTALL_ROOT="${pkgdir}" install
   cd ..

   cd notifier
   make INSTALL_ROOT="${pkgdir}" install
   cd ..
   
   cd repoeditor
   make INSTALL_ROOT="${pkgdir}" install
   cd ..
   
   cd cachecleaner
   make INSTALL_ROOT="${pkgdir}" install
   cd ..

   cd sudo
   make INSTALL_ROOT="${pkgdir}" install
}
