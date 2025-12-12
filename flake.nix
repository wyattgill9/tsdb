{
  description = "C++ development environment";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  };

  outputs = {
    nixpkgs,
    ...
  }: let
    systems = ["x86_64-linux" "aarch64-darwin"];
    eachSystem = f:
      nixpkgs.lib.genAttrs systems (system:
        f {
          inherit system;
          pkgs = nixpkgs.legacyPackages.${system};
          toolchain = nixpkgs.legacyPackages.${system}.llvmPackages_latest;
        });
  in {
    packages = eachSystem ({
      pkgs,
      toolchain,
      ...
    }: {
      default = import ./package.nix {inherit pkgs toolchain;};
    });

    devShells = eachSystem ({
      pkgs,
      toolchain,
      ...
    }: {
      default = import ./shell.nix {inherit pkgs toolchain;};
    });
  };
}
