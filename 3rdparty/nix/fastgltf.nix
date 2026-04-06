{
  lib,
  pkgs,
  stdenv,
  cmake,
  src,
  version ? "0.9.0",
}:

stdenv.mkDerivation rec {
  pname = "fastgltf";
  inherit version;
  inherit src;
  nativeBuildInputs = [ cmake ];
  buildInputs = [
    pkgs.eigen_3_4_0
    pkgs.simdjson
  ];

  cmakeFlags = [
    "-DFASTGLTF_COMPILE_AS_CPP20=ON"
  ];

  meta = with lib; {
    description = "fastgltf is a speed and usability focused glTF 2.0 library written in modern C++17 with minimal dependencies.";
    homepage = "https://github.com/spnda/fastgltf";
    license = licenses.mit;
    platforms = platforms.all;
    maintainers = [ ];
  };
}
