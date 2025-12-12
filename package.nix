{
  pkgs,
  toolchain,
  ...
}:
toolchain.stdenv.mkDerivation {
  pname = "my-cpp-project";
  version = "0.1.0";

  src = ./.;

  nativeBuildInputs = with pkgs; [
    cmake
    ninja
  ];
}
