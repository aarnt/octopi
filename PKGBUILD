_pkgname=octopi
pkgname=octopi-git
pkgver=0.16.2.latest
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
   cp resources/images/octopi_green.png resources/images/octopi.png
}

pkgver() {
   cd "${_pkgname}"
   git describe --long --tags --abbrev=7 | sed 's/\([^-]*-g\)/r\1/;s/-/./g;s/^v//'
}

build() {
   cd "${_pkgname}"
   echo "Starting build..."   
   qmake6 PREFIX=/usr QMAKE_CFLAGS="${CFLAGS}" QMAKE_CXXFLAGS="${CXXFLAGS}" QMAKE_LFLAGS="${LDFLAGS}" octopi.pro
   make

   _subdirs="cachecleaner helper notifier repoeditor"

   for _subdir in $_subdirs; do
     pushd $_subdir
     echo "Building octopi-$_subdir..."
     qmake6 PREFIX=/usr QMAKE_CFLAGS="${CFLAGS}" QMAKE_CXXFLAGS="${CXXFLAGS}" QMAKE_LFLAGS="${LDFLAGS}" "octopi-$_subdir.pro"
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
