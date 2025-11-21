{
  # Maintenance: Run 'nix flake update' to update nixpkgs. For alpm_octopi_utils and qt-sudo,
  # check GitHub for new commits/tags and update rev + sha256 (get hash from error output).
  description = "Octopi Nix development environment";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = {
    self,
    nixpkgs,
    flake-utils,
  }:
    flake-utils.lib.eachDefaultSystem (
      system: let
        pkgs = nixpkgs.legacyPackages.${system};

        alpm_octopi_utils = pkgs.stdenv.mkDerivation {
          pname = "alpm_octopi_utils";
          version = "unstable-2024-12-27";

          src = pkgs.fetchFromGitHub {
            owner = "aarnt";
            repo = "alpm_octopi_utils";
            rev = "0ed2a8bd6b869f40683cf7a79727dc64d7da274e";
            sha256 = "sha256-TxjXtdhp2BqkGDVYnyhQ5mccU0MDndFfAZMuBQRVf1c=";
          };

          nativeBuildInputs = with pkgs; [cmake vala pkg-config];
          buildInputs = with pkgs; [pacman glib libarchive];
        };

        qt-sudo = pkgs.stdenv.mkDerivation {
          pname = "qt-sudo";
          version = "2.2.0";

          src = pkgs.fetchFromGitHub {
            owner = "aarnt";
            repo = "qt-sudo";
            rev = "v2.2.0";
            sha256 = "sha256-bvRQEOHMnmAr95ecHkVD0JWGiCMTCx51ncMwrS3/wu0=";
          };

          nativeBuildInputs = with pkgs; [qt6.wrapQtAppsHook qt6.qmake qt6.qttools];
          buildInputs = with pkgs; [qt6.qtbase];

          postConfigure = ''
            substituteInPlace Makefile \
              --replace-quiet "${pkgs.qt6.qtbase}/bin/lrelease" "${pkgs.qt6.qttools}/bin/lrelease"
          '';
        };
      in {
        devShells.default = pkgs.mkShell {
          buildInputs = with pkgs; [
            alpm_octopi_utils
            cmake
            gcc
            git
            glib
            gnumake
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
            sudo
            vala
          ];

          shellHook = ''
            export OCTOPI_ALLOWED_PATHS="/usr/bin:$PWD/build"
            export LD_LIBRARY_PATH="${pkgs.pipewire}/lib:$LD_LIBRARY_PATH"
          '';

          QT_QPA_PLATFORM_PLUGIN_PATH = "${pkgs.qt6.qtbase}/${pkgs.qt6.qtbase.qtPluginPrefix}";
          QT_PLUGIN_PATH = "${pkgs.qt6.qtbase}/${pkgs.qt6.qtbase.qtPluginPrefix}";
        };
      }
    );
}
