{
  description = "Crystal Explorer";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
  };

  outputs =
    { self, nixpkgs }:
    let
      system = "x86_64-linux";
      pkgs = nixpkgs.legacyPackages.${system};
      qtEnv = pkgs.qt6.env "qt6-simc-${pkgs.qt6.qtbase.version}" [
        pkgs.qt6.qtbase
        pkgs.qt6.qttools
        pkgs.qt6.qtdeclarative
        pkgs.qt6.qt5compat
        pkgs.qt6.qtwebchannel
        pkgs.qt6.qtpositioning
      ];
      fastgltf = pkgs.callPackage ./3rdparty/nix/fastgltf.nix { };
      gemmi = pkgs.callPackage ./3rdparty/nix/gemmi.nix { };
      CrystalExplorer = pkgs.stdenv.mkDerivation {
        name = "CrystalExplorer";
        src = ./.;
        buildInputs = [
          pkgs.cmake
          pkgs.cpm-cmake

          pkgs.eigen_3_4_0
          pkgs.unordered_dense
          pkgs.fmt
          pkgs.nlohmann_json
          pkgs.simdjson
          fastgltf
          gemmi

          # qt
          qtEnv
          pkgs.qt6.qtbase
          pkgs.qt6.wrapQtAppsHook

          pkgs.makeWrapper
          pkgs.xorg.libXcursor
          pkgs.xorg.libxcb
          pkgs.xorg.xcbutilcursor # <--- This fixes the "xcb-cursor0" error
          pkgs.libxkbcommon
        ];
        nativeBuildInputs = [
          pkgs.cmake
          pkgs.ninja
          pkgs.pkg-config
          pkgs.ccache
        ];
        env = {
          CCACHE_DIR = "/var/cache/ccache";
          CCACHE_BASEDIR = "$NIX_BUILD_TOP";
          CCACHE_SLOPPINESS = "locale,time_macros";
        };
        cmakeFlags = [
          "-GNinja"
          "-DCPM_DOWNLOAD_LOCATION=${pkgs.cpm-cmake}/share/cpm/CPM.cmake"
          "-DUSE_SYSTEM_LIBXC=ON"
          "-DCPM_USE_LOCAL_PACKAGES=ON"
          "-DCMAKE_C_COMPILER_LAUNCHER=${pkgs.ccache}/bin/ccache"
          "-DCMAKE_CXX_COMPILER_LAUNCHER=${pkgs.ccache}/bin/ccache"
          "-DNIX_BUILD=ON"
        ];
        qtWrapperArgs = [
          "--set QT_QPA_PLATFORM wayland;xcb"
          "--set WAYLAND_DISPLAY \"$WAYLAND_DISPLAY\""
          "--set XDG_RUNTIME_DIR \"$XDG_RUNTIME_DIR\""
        ];
        NIX_LDFLAGS = "-lquadmath";
        postInstall = ''
          mkdir -p $out/bin
          cp src/CrystalExplorer $out/bin/
        '';
      };
      cexp-desktop = pkgs.makeDesktopItem {
        name = "crystalexplorer";
        desktopName = "CrystalExplorer";
        icon = ./icons/CrystalExplorer512x512.png;
        comment = "Crystal Explorer";
        exec = "CrystalExplorer";
        categories = [
          "X-Cristallography"
          "Science"
          "Education"
        ];
        terminal = true;
      };
    in
    {
      packages.${system}.default = pkgs.symlinkJoin {
        name = "olex2-wrapper";
        paths = [
          CrystalExplorer
          cexp-desktop
        ];
      };
      devShells.${system}.default = pkgs.mkShell {
        nativeBuildInputs = with pkgs; [
          pkg-config
          cmake
          ninja
          pkgs.qt6.wrapQtAppsHook
        ];

        buildInputs = with pkgs; [
          zlib
          openssl
          libGL
          qt6.qtbase
          qt6.qtsvg
          qt6.qttools
          qt6.qtgraphs
          xorg.libX11
        ];

        shellHook = ''
          echo "🛠️  Build environment ready for x86_64-linux"
          echo "   Compilers: $(gcc --version | head -n1)"
        '';
      };
    };
}
