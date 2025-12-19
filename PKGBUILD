# Maintainer:
# Contributor: Mark Wagie <mark dot wagie at proton dot me>
# Contributor: MatMoul <matmoul at the google email domain which is .com>

: ${_commit_alpm=789ba9acc52b7b0624fb08de9f4756b5d51c10f1}

_pkgname="octopi"
pkgname="$_pkgname"
pkgver=git
pkgrel=1
pkgdesc="A powerful Pacman frontend using Qt libs"
url="https://github.com/aarnt/octopi"
license=('GPL-2.0-or-later')
arch=('x86_64')

depends=(
  'kstatusnotifieritem'
  'qt6-multimedia'
  'qtermwidget'
)
makedepends=(
  'cmake'
  'git'
  'ninja'
  'qt6-5compat'
  'qt6-tools'
  'vala'
)
optdepends=(
  'inxi: for SysInfo log'
  'lsb-release: for SysInfo log'
  'mhwd: for SysInfo log'
  'pacaur: for AUR support'
  'pacmanlogviewer: to view pacman log files'
  'paru: for AUR support'
  'pikaur: for AUR support'
  'systemd: for SysInfo log'
  'trizen: for AUR support'
  'yay: for AUR support'
)

_source_octopi() {
  _pkgsrc_octopi="$_pkgname-$pkgver"
  local _pkgext="tar.gz"
  source+=("$_pkgsrc_octopi"::"git+https://github.com/aarnt/octopi.git")
  sha256sums+=('SKIP')

  _build_octopi() (
    local _cmake_options=(
      -B build_octopi
      -S "$_pkgsrc_octopi"
      -G Ninja
      -DCMAKE_BUILD_TYPE=None
      -DCMAKE_INSTALL_PREFIX='/usr'
      -Dalpm_octopi_utils_DIR="'$srcdir/fakeinstall/usr/lib/cmake/alpm_octopi_utils/'"
      -Wno-dev
    )

    cmake "${_cmake_options[@]}"
    cmake --build build_octopi
  )
}

_source_alpm_utils() {
  conflicts+=('alpm_octopi_utils')

  _pkgsrc_alpm_utils="alpm_octopi_utils"
  source+=("$_pkgsrc_alpm_utils"::"git+https://github.com/aarnt/alpm_octopi_utils.git${_commit_alpm:+#commit=$_commit_alpm}")
  sha256sums+=('SKIP')

  _build_alpm_utils() (
    local _cmake_options=(
      -B build_alpm
      -S "$_pkgsrc_alpm_utils"
      -G Ninja
      -DCMAKE_BUILD_TYPE=None
      -DCMAKE_INSTALL_PREFIX='/usr'
      -Wno-dev
    )

    cmake "${_cmake_options[@]}"
    cmake --build build_alpm

    DESTDIR="fakeinstall" cmake --install build_alpm
  )
}

_source_octopi
_source_alpm_utils

prepare() {
  _run_if_exists _prepare_octopi
  _run_if_exists _prepare_alpm_utils
}

build() {
  _run_if_exists _build_alpm_utils
  _run_if_exists _build_octopi
}

package() {
  depends+=(
    'pacman'
    'pacman-contrib'
    'qt-sudo' # AUR
  )

  DESTDIR="$pkgdir" cmake --install build_octopi

  # library
  install -Dm644 "fakeinstall/usr/lib/libalpm_octopi_utils.so" -t "$pkgdir/usr/lib/"

  # not needed for standard licenses
  rm -rf "$pkgdir/usr/share/licenses/"
}

_run_if_exists() {
  if declare -F "$1" > /dev/null; then
    eval "$1"
  fi
}
