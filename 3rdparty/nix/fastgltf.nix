{
  lib,
  pkgs,
  stdenv,
  fetchFromGitHub,
  cmake,
}:

stdenv.mkDerivation rec {
  pname = "fastgltf";
  version = "0.9.0";

  src = fetchFromGitHub {
    owner = "spnda";
    repo = "fastgltf";
    rev = "v${version}";
    hash = "sha256-15hFpOZdCo1I9+XJpbtEk3sqE19IO67PpbnpE4RnbBU=";
  };

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
