{
  lib,
  stdenv,
  fetchFromGitHub,
  cmake,
  zlib,
}:

stdenv.mkDerivation (finalAttrs: {
  pname = "gemmi";
  version = "0.6.7";

  src = fetchFromGitHub {
    owner = "project-gemmi";
    repo = "gemmi";
    tag = "v${finalAttrs.version}";
    hash = "sha256-Y7gQSh9C7smoXuGWgpJI3hPIg06Jns+1dBpmMxuCrKE=";
  };

  nativeBuildInputs = [
    cmake
  ];
  buildInputs = [ zlib ];

  cmakeFlags = [
    "DUSE_PYTHON=OFF"
    "BUILD_SHARED_LIBS=OFF"
    "BUILD_GEMMI_PROGRAM=OFF"
  ];

  doCheck = false;

  pythonImportsCheck = [ "gemmi" ];

  disabledTests = lib.optionals (stdenv.hostPlatform.isLinux && stdenv.hostPlatform.isAarch64) [
    # Numerical precision error
    # self.assertTrue(numpy.allclose(data_f, abs(asu_val), atol=5e-5, rtol=0))
    # AssertionError: False is not true
    "test_reading"
  ];

  enabledTestPaths = [ "../tests" ];

  meta = {
    description = "Macromolecular crystallography library and utilities";
    homepage = "https://github.com/project-gemmi/gemmi";
    changelog = "https://github.com/project-gemmi/gemmi/releases/tag/v${finalAttrs.version}";
    license = lib.licenses.mpl20;
    maintainers = with lib.maintainers; [ natsukium ];
    mainProgram = "gemmi";
    platforms = lib.platforms.unix;
  };
})
