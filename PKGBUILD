pkgname=octopi
pkgver=0.2
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

build() {
    cpucount=$(grep -c processor /proc/cpuinfo 2>/dev/null)
    jc=$((${cpucount:-1}))

    #cd "$srcdir"  
    msg "Starting build..."

    cd $startdir
    qmake-qt4 $pkgname.pro 	
    make -j $jc

    cd $startdir/notifier/pacmanhelper
    msg "Building pacmanhelper..."
    qmake-qt4 pacmanhelper.pro
    make -j $jc    
    
    cd $startdir/notifier/octopi-notifier
    msg "Building octopi-notifier..."
    qmake-qt4 octopi-notifier.pro
    make -j $jc
}

package() {   
   #Octopi main files
   install -D -m755 $startdir/bin/$pkgname ${pkgdir}/usr/bin/$pkgname
   install -D -m644 $startdir/$pkgname.desktop ${pkgdir}/usr/share/applications/$pkgname.desktop
   install -D -m644 $startdir/resources/images/${pkgname}_yellow.png ${pkgdir}/usr/share/icons/$pkgname.png

   #Pacmanhelper service files
   install -D -m755 $startdir/notifier/bin/pacmanhelper ${pkgdir}/usr/lib/octopi/pacmanhelper

   install -D -m644 $startdir/notifier/pacmanhelper/polkit/org.octopi.pacman.policy ${pkgdir}/usr/share/polkit-1/actions/org.octopi.pacman.policy
   install -D -m644 $startdir/notifier/pacmanhelper/polkit/org.octopi.pacmanhelper.conf ${pkgdir}/etc/dbus-1/system.d/org.octopi.pacmanhelper.conf
   install -D -m644 $startdir/notifier/pacmanhelper/polkit/org.octopi.pacmanhelper.xml ${pkgdir}/usr/share/dbus-1/interfaces/org.octopi.pacmanhelper.xml
   install -D -m644 $startdir/notifier/pacmanhelper/polkit/org.octopi.pacmanhelper.service ${pkgdir}/usr/share/dbus-1/system-services/org.octopi.pacmanhelper.service

   #Octopi-notifier file
   install -D -m755 $startdir/notifier/bin/octopi-notifier ${pkgdir}/usr/bin/octopi-notifier
}
