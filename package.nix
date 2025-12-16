{
  pkgs,
  lib,
  seastar,
  ...
}:

let
  llvm = pkgs.llvmPackages_latest;
in
llvm.stdenv.mkDerivation {
  pname = "my-cpp-project";
  version = "0.1.0";

  src = lib.sourceByRegex ./. [
    "^src.*"
    "^include.*"
    "^CMakeLists.txt"
  ];

  nativeBuildInputs = with pkgs; [
    cmake
    ninja
    pkg-config
    capnproto
    llvm.clang-tools
  ];

  buildInputs = [
    seastar
    pkgs.abseil-cpp
    pkgs.glaze
    # pkgs.boost
  ];

  cmakeFlags = [
    "-DCMAKE_C_COMPILER=${llvm.stdenv.cc}/bin/clang"
    "-DCMAKE_CXX_COMPILER=${llvm.stdenv.cc}/bin/clang++"
  ];

  shellHook = ''
    export CC=${llvm.stdenv.cc}/bin/clang
    export CXX=${llvm.stdenv.cc}/bin/clang++
  '';
}
