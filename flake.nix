{
  description = "Octopi development environment";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs =
    { nixpkgs, flake-utils, ... }:
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = nixpkgs.legacyPackages.${system};

        alpm_octopi_utils = pkgs.stdenv.mkDerivation {
          pname = "alpm_octopi_utils";
          version = "unstable-2024-12-27";

          src = pkgs.fetchFromGitHub {
            owner = "aarnt";
            repo = "alpm_octopi_utils";
            rev = "0ed2a8bd6b869f40683cf7a79727dc64d7da274e";
            hash = "sha256-TxjXtdhp2BqkGDVYnyhQ5mccU0MDndFfAZMuBQRVf1c=";
          };

          nativeBuildInputs = with pkgs; [
            cmake
            vala
            pkg-config
          ];
          buildInputs = with pkgs; [
            pacman
            glib
            libarchive
          ];
        };

        qt-sudo = pkgs.stdenv.mkDerivation {
          pname = "qt-sudo";
          version = "2.4.0";

          src = pkgs.fetchFromGitHub {
            owner = "aarnt";
            repo = "qt-sudo";
            rev = "v2.4.0";
            hash = "sha256-s7kRYKCgUgEScZVsyEmzSiRCUJ+30t/OIkqos8tMXo8=";
          };

          nativeBuildInputs = with pkgs; [
            qt6.wrapQtAppsHook
            qt6.qmake
            qt6.qttools
          ];
          buildInputs = [ pkgs.qt6.qtbase ];

          postConfigure = ''
            substituteInPlace Makefile \
              --replace-quiet "${pkgs.qt6.qtbase}/bin/lrelease" "${pkgs.qt6.qttools}/bin/lrelease"
          '';
        };
      in
      {
        formatter = pkgs.nixfmt;

        devShells.default = pkgs.mkShell {
          packages = with pkgs; [
            alpm_octopi_utils
            cmake
            gcc
            git
            glib
            gnumake
            kdePackages.kstatusnotifieritem
            libarchive
            lxqt.qtermwidget
            pacman
            pipewire
            pkg-config
            qt-sudo
            qt6.qmake
            qt6.qt5compat
            qt6.qtbase
            qt6.qtmultimedia
            qt6.qttools
            qt6.wrapQtAppsHook
            vala
          ];

          env = {
            QT_QPA_PLATFORM_PLUGIN_PATH = "${pkgs.qt6.qtbase}/${pkgs.qt6.qtbase.qtPluginPrefix}";
            QT_PLUGIN_PATH = "${pkgs.qt6.qtbase}/${pkgs.qt6.qtbase.qtPluginPrefix}";
          };

          shellHook = ''
            export OCTOPI_ALLOWED_PATHS="/usr/bin:$PWD/build"
            export LD_LIBRARY_PATH="${pkgs.pipewire}/lib:$LD_LIBRARY_PATH"

            # nixpkgs supplies libalpm headers for the build, but the host
            # pacman writes the on-disk DB and the host sudo carries the
            # setuid bit, so a small set of host tools must take priority
            # at runtime.
            shim_dir="$(mktemp -d -t octopi-host-shims-XXXXXX)"
            for tool in sudo pacman fakeroot pacman-conf; do
              [ -x "/usr/bin/$tool" ] && ln -sf "/usr/bin/$tool" "$shim_dir/$tool"
            done
            export PATH="$shim_dir:$PATH"
          '';
        };
      }
    );
}
