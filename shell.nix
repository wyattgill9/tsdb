{
  pkgs,
  toolchain,
  ...
}:
pkgs.mkShell.override {
  stdenv = toolchain.stdenv;
} {
  buildInputs = with pkgs; [
    cmake
    ninja
    toolchain.clang
  ];

  CC = "${toolchain.clang}/bin/clang";
  CXX = "${toolchain.clang}/bin/clang++";
}
