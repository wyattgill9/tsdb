{
  description = "C++ development environment";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    seastar.url = "github:wyattgill9/seastar-flake";
  };

  outputs = {
    self,
    nixpkgs,
    seastar,
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
      system,
      ...
    }: {
      default = pkgs.callPackage ./package.nix {
        seastar = seastar.packages.${system}.default;
      };
    });

    devShells = eachSystem ({system, ...}: {
      default = (self.packages.${system} or {}).default;
    });
  };
}
