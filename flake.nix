{
  description = "C++ development environment";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  };

  outputs = {
    self,
    nixpkgs,
    ...
  }: let
    systems = ["x86_64-linux" "aarch64-darwin"];
    eachSystem = f:
      nixpkgs.lib.genAttrs systems (system:
        f {
          inherit system;
          pkgs = nixpkgs.legacyPackages.${system};
        });
  in {
    packages = eachSystem ({
      pkgs,
      ...
    }: {
      default = pkgs.callPackage ./package.nix {};
    });

    devShells = eachSystem ({system, ...}: {
      default = (self.packages.${system} or {}).default;
    });
  };
}
