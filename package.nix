{
  pkgs,
  lib,
  ...
}:

# TODO MAKE SHELL CLANG STENV
let
  toolchain = pkgs.llvmPackages_latest.stdenv;
in
toolchain.mkDerivation {
  pname = "my-cpp-project";
  version = "0.1.0";

  src = lib.sourceByRegex ./. [
    "^src.*"
    "^include.*"
    "^CMakeLists.txt"
  ];
  nativeBuildInputs = [
    pkgs.cmake
    pkgs.ninja
  ];

  cmakeFlags = [
    "-DCMAKE_C_COMPILER=${toolchain.cc}/bin/clang"
    "-DCMAKE_CXX_COMPILER=${toolchain.cc}/bin/clang++"
  ];

  # Runtime dependencies would go in buildInputs
  # buildInputs = [];
}
