
pkgname=octopi
pkgver=0.8.1
pkgrel=1
pkgdesc="This is Octopi, a powerful Pacman frontend using Qt libs"
url="https://octopiproject.wordpress.com/"
arch=('i686' 'x86_64')
license=('GPL3')
depends=('qt5-quickcontrols' 'pacman' 'pkgfile' 'knotifications' 'alpm_octopi_utils')
makedepends=('cmake' 'asciidoc')
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
   cd ${pkgname}-${pkgver}/

   # enable the kstatus switch, disable if you wish to build without Plasma/knotifications support
   sed -e "s|# DEFINES += KSTATUS| DEFINES += KSTATUS|" -i notifier/octopi-notifier/octopi-notifier.pro
   # enable alpm backend, disable if you wish to build without alpm_octopi_utils
   sed -e "s|#ALPM_BACKEND|ALPM_BACKEND|" -i octopi.pro
}
         
build() {
   cd ${pkgname}-${pkgver}/
   
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
   cd ${pkgname}-${pkgver}/
   
   #Octopi main files
   install -D -m755 bin/$pkgname ${pkgdir}/usr/bin/$pkgname
   install -D -m644 $pkgname.desktop ${pkgdir}/usr/share/applications/$pkgname.desktop
   install -D -m644 resources/images/${pkgname}_green.png ${pkgdir}/usr/share/icons/$pkgname.png
   install -D -m644 resources/images/${pkgname}_green.png ${pkgdir}/usr/share/icons/gnome/32x32/apps/$pkgname.png
   install -D -m644 resources/images/${pkgname}_red.png ${pkgdir}/usr/share/icons/${pkgname}_red.png
   install -D -m644 resources/images/${pkgname}_yellow.png ${pkgdir}/usr/share/icons/${pkgname}_yellow.png
   
   #speedup files
   install -D -m755 speedup/speedup-octopi.sh ${pkgdir}/usr/bin/speedup-octopi.sh
   install -D -m644 speedup/${pkgname}.service ${pkgdir}/etc/systemd/system/${pkgname}.service

   #Pacmaneditor files
   install -D -m755 repoeditor/bin/octopi-repoeditor ${pkgdir}/usr/bin/octopi-repoeditor
   
   #Pacmanhelper service files
   install -D -m755 notifier/bin/pacmanhelper ${pkgdir}/usr/lib/octopi/pacmanhelper

   install -D -m644 notifier/pacmanhelper/polkit/org.octopi.pacman.policy ${pkgdir}/usr/share/polkit-1/actions/org.octopi.pacman.policy
   install -D -m644 notifier/pacmanhelper/polkit/org.octopi.pacmanhelper.conf ${pkgdir}/etc/dbus-1/system.d/org.octopi.pacmanhelper.conf
   install -D -m644 notifier/pacmanhelper/polkit/org.octopi.pacmanhelper.xml ${pkgdir}/usr/share/dbus-1/interfaces/org.octopi.pacmanhelper.xml
   install -D -m644 notifier/pacmanhelper/polkit/org.octopi.pacmanhelper.service ${pkgdir}/usr/share/dbus-1/system-services/org.octopi.pacmanhelper.service
   
   #Octopi-notifier file
   install -D -m755 notifier/bin/octopi-notifier ${pkgdir}/usr/bin/octopi-notifier
   install -D -m644 octopi-notifier.desktop ${pkgdir}/etc/xdg/autostart/octopi-notifier.desktop
   
   #Octopi-repoeditor file
   install -D -m755 repoeditor/bin/octopi-repoeditor ${pkgdir}/usr/bin/octopi-repoeditor
   
   #Octopi-cachecleaner file
   install -D -m755 cachecleaner/bin/octopi-cachecleaner ${pkgdir}/usr/bin/octopi-cachecleaner
   install -D -m644 cachecleaner/octopi-cachecleaner.desktop ${pkgdir}/usr/share/applications/octopi-cachecleaner.desktop

}
