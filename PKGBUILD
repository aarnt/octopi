
pkgname=octopi
pkgver=0.8.1
pkgrel=1
pkgdesc="This is Octopi, a powerful Pacman frontend using Qt libs"
url="https://octopiproject.wordpress.com/"
arch=('i686' 'x86_64')
license=('GPL3')
depends=('qt5-quickcontrols' 'pacman' 'pkgfile' 'knotifications' 'alpm_octopi_utils')
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
   cd ${pkgname}
   
   make INSTALL_ROOT=${pkgdir}/ install
}
