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
    in
    {
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
