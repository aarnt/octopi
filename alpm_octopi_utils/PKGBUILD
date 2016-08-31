pkgname=('libalpm_octopi_utils')
pkgver=1.0.0
pkgrel=1
pkgdesc="Alpm utils for Octopi"
arch=('i686' 'x86_64')
url="http://octopiproject.wordpress.com"
license=('GPL2')
install=$pkgname.install
makedepends=('vala')
depends=()
optdepends=()
provides=('libalpm_octopi_utils')
md5sums=()

build() {
    cpucount=$(grep -c processor /proc/cpuinfo 2>/dev/null)
    jc=$((${cpucount:-1}))

    #cd "$srcdir"  
    msg "Starting build..." alpm_octopi_utils.h

    cd $startdir
    make -j $jc
}

package_libalpm_octopi_utils() {   
    install -D -m755 $startdir/src/$pkgname.so ${pkgdir}/usr/lib/$pkgname.so
    install -D -m644 $startdir/src/$pkgname.pc ${pkgdir}/usr/lib/pkgconfig/$pkgname.pc
    install -D -m644 $startdir/src/alpm_octopi_utils.h ${pkgdir}/usr/include/alpm_octopi_utils.h   
}
