pkgname=octopi
pkgver=0.1.7.2
pkgrel=1
pkgdesc="a powerful Pacman frontend using Qt libs"
arch=('i686' 'x86_64')
url="http://octopiproject.wordpress.com"
license=('GPL2')
install=$pkgname.install
makedepends=('git')
depends=('qt4')
provides=('octopi')
conflicts=('octopi')
md5sums=()
_gitroot="git://github.com/aarnt/octopi.git"
_gitname="octopi"
_projpath="$HOME/Projects/$pkgname"

build() {
     cpucount=$(grep -c processor /proc/cpuinfo 2>/dev/null)
  jc=$((${cpucount:-1}))

    #cd "$srcdir"
   
    msg "Starting build..."

    qmake-qt4 $_projpath/$pkgname.pro
    #"CONFIG+=LINUX_INTEGRATED" \
    #"INSTALL_ROOT_PATH=$pkgdir/usr/" \
    #"LOWERED_APPNAME=$pkgname"
    
    make -j $jc
}

package() {
   # cd "$srcdir/$pkgname"
 
   install -D -m755 $_projpath/bin/$pkgname ${pkgdir}/usr/bin/$pkgname
   install -D -m644 $_projpath/$pkgname.desktop ${pkgdir}/usr/share/applications/$pkgname.desktop
   install -D -m644 $_projpath/resources/images/${pkgname}_yellow.png ${pkgdir}/usr/share/icons/$pkgname.png

    #make DESTDIR="${pkgdir}" install
    #make INSTALL_ROOT=${pkgdir} install
}
