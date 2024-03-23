_pkgname=octopi
pkgname=octopi-git
pkgver=0.15.0.r15.g69e85dd
pkgrel=1
pkgdesc="This is Octopi, a powerful Pacman frontend using Qt libs (git checkout)"
url="https://tintaescura.com/projects/octopi/"
arch=('i686' 'x86_64')
license=('GPL2')
depends=('alpm_octopi_utils' 'qtermwidget' 'sudo')
makedepends=('git')
groups=('system')
source=("git+https://github.com/aarnt/octopi.git")
md5sums=('SKIP')

prepare() {
   cd "${_pkgname}"
   
   # enable the kstatus switch, disable if you wish to build without Plasma/knotifications support
   sed -e "s|DEFINES += OCTOPI_EXTENSIONS ALPM_BACKEND #KSTATUS|DEFINES += OCTOPI_EXTENSIONS ALPM_BACKEND KSTATUS|" -i notifier/octopi-notifier.pro
      
   cp resources/images/octopi_green.png resources/images/octopi.png
}

pkgver() {
   cd "${_pkgname}"
   git describe --long --tags --abbrev=7 | sed 's/\([^-]*-g\)/r\1/;s/-/./g;s/^v//'
}
         
build() {
   cd "${_pkgname}"
   echo "Starting build..."   
   qmake-qt5 PREFIX=/usr QMAKE_CFLAGS="${CFLAGS}" QMAKE_CXXFLAGS="${CXXFLAGS}" QMAKE_LFLAGS="${LDFLAGS}" octopi.pro
   make

   _subdirs="cachecleaner helper notifier repoeditor"

   for _subdir in $_subdirs; do
     pushd $_subdir
     echo "Building octopi-$_subdir..."
     qmake-qt5 PREFIX=/usr QMAKE_CFLAGS="${CFLAGS}" QMAKE_CXXFLAGS="${CXXFLAGS}" QMAKE_LFLAGS="${LDFLAGS}" "octopi-$_subdir.pro"
     make
     popd
   done  
}

package() {
   cd "${_pkgname}"
   make INSTALL_ROOT="${pkgdir}" install

   _subdirs="cachecleaner helper notifier repoeditor"

   for _subdir in $_subdirs; do
     pushd $_subdir
     make INSTALL_ROOT="${pkgdir}" install
     popd
   done   
}
