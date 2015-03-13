pkgname=('octopi' 'octopi-notifier' 'octopi-repoeditor' 'octopi-cachecleaner')
pkgver=0.7.0
pkgrel=1
pkgdesc="a powerful Pacman frontend using Qt libs"
arch=('i686' 'x86_64')
url="http://octopiproject.wordpress.com"
license=('GPL2')
install=$pkgname.install
makedepends=('git')
depends=('qt5-base' 'qt5-declarative' 'xterm')
optdepends=('kdesu: for KDE'
            'gksu: for XFCE, Gnome, LXDE, Cinnamon'
            'gnome-keyring: for password management'
            'yaourt: for AUR support'
            'octopi-notifier: for notifications'
            'pkgfile: to view uninstalled pkg contents in ArchLinux')
provides=('octopi')
md5sums=()

build() {
    cpucount=$(grep -c processor /proc/cpuinfo 2>/dev/null)
    jc=$((${cpucount:-1}))

    #cd "$srcdir"  
    msg "Starting build..."

    cd $startdir
    qmake-qt5 $pkgname.pro 	
    make -j $jc

    cd $startdir/notifier/pacmanhelper
    msg "Building pacmanhelper..."
    qmake-qt5 pacmanhelper.pro
    make -j $jc    
    
    cd $startdir/notifier/octopi-notifier
    msg "Building octopi-notifier..."
    qmake-qt5 octopi-notifier.pro
    make -j $jc

    cd $startdir/repoeditor
    msg "Building octopi-repoeditor..."
    qmake-qt5 octopi-repoeditor.pro
    make -j $jc

    cd $startdir/cachecleaner
    msg "Building octopi-cachecleaner..."
    qmake-qt5 octopi-cachecleaner.pro
    make -j $jc

}

package_octopi() {   
   #Octopi main files
   install -D -m755 $startdir/bin/$pkgname ${pkgdir}/usr/bin/$pkgname
   install -D -m644 $startdir/$pkgname.desktop ${pkgdir}/usr/share/applications/$pkgname.desktop
   install -D -m644 $startdir/resources/images/${pkgname}_green.png ${pkgdir}/usr/share/icons/$pkgname.png
   install -D -m644 $startdir/resources/images/${pkgname}_green.png ${pkgdir}/usr/share/icons/gnome/32x32/apps/$pkgname.png
   install -D -m644 $startdir/resources/images/${pkgname}_red.png ${pkgdir}/usr/share/icons/${pkgname}_red.png
   install -D -m644 $startdir/resources/images/${pkgname}_yellow.png ${pkgdir}/usr/share/icons/${pkgname}_yellow.png

   #speedup files
   install -D -m755 $startdir/speedup/speedup-octopi.sh ${pkgdir}/usr/bin/speedup-octopi.sh
   install -D -m644 $startdir/speedup/${pkgname}.service ${pkgdir}/etc/systemd/system/${pkgname}.service

   #Pacmaneditor files
   install -D -m755 $startdir/repoeditor/bin/octopi-repoeditor ${pkgdir}/usr/bin/octopi-repoeditor

   #Cachecleaner files
   install -D -m755 $startdir/cachecleaner/bin/octopi-cachecleaner ${pkgdir}/usr/bin/octopi-cachecleaner
   install -D -m644 $startdir/cachecleaner/octopi-cachecleaner.desktop ${pkgdir}/usr/share/applications/octopi-cachecleaner.desktop
}

package_octopi-notifier() {
   pkgdesc="Notifier for Octopi"
   depends=('libnotify')
   optdepends=('octopi: launch graphical package manager from tray'
               'xfce4-notifyd: for notifications in XFCE')

   #Pacmanhelper service files
   install -D -m755 $startdir/notifier/bin/pacmanhelper ${pkgdir}/usr/lib/octopi/pacmanhelper

   install -D -m644 $startdir/notifier/pacmanhelper/polkit/org.octopi.pacman.policy ${pkgdir}/usr/share/polkit-1/actions/org.octopi.pacman.policy
   install -D -m644 $startdir/notifier/pacmanhelper/polkit/org.octopi.pacmanhelper.conf ${pkgdir}/etc/dbus-1/system.d/org.octopi.pacmanhelper.conf
   install -D -m644 $startdir/notifier/pacmanhelper/polkit/org.octopi.pacmanhelper.xml ${pkgdir}/usr/share/dbus-1/interfaces/org.octopi.pacmanhelper.xml
   install -D -m644 $startdir/notifier/pacmanhelper/polkit/org.octopi.pacmanhelper.service ${pkgdir}/usr/share/dbus-1/system-services/org.octopi.pacmanhelper.service

   #Octopi-notifier file
   install -D -m755 $startdir/notifier/bin/octopi-notifier ${pkgdir}/usr/bin/octopi-notifier
   install -D -m644 $startdir/octopi-notifier.desktop ${pkgdir}/etc/xdg/autostart/octopi-notifier.desktop
}

package_octopi-repoeditor() {
   pkgdesc="Repoeditor for Octopi"
   
   #Octopi-repoeditor file
   install -D -m755 $startdir/repoeditor/bin/octopi-repoeditor ${pkgdir}/usr/bin/octopi-repoeditor
}

package_octopi-cachecleaner() {
   pkgdesc="Cachecleaner for Octopi"

   #Octopi-cachecleaner file
   install -D -m755 $startdir/cachecleaner/bin/octopi-cachecleaner ${pkgdir}/usr/bin/octopi-cachecleaner
   install -D -m644 $startdir/cachecleaner/$pkgname.desktop ${pkgdir}/usr/share/applications/$pkgname.desktop
}
