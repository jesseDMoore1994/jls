{
  description = "A template for Nix based C project setup.";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs";
    utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, ... }@inputs: inputs.utils.lib.eachSystem [
    "x86_64-linux" "i686-linux" "aarch64-linux" "x86_64-darwin"
  ] (system: let pkgs = import nixpkgs {
        inherit system;
      };
      jls = pkgs.stdenv.mkDerivation {
        pname = "jls";
        version = "0.1.0";
        src = ./.;
        nativeBuildInputs = [
          pkgs.gnumake
          pkgs.gcc
        ];
        buildPhase = "make jls";
        installPhase = ''
          mkdir -p $out/bin
          mv jls $out/bin
        '';
      };
  in rec {
    defaultApp = inputs.utils.lib.mkApp { drv = defaultPackage; };
    defaultPackage = jls;
    devShell = pkgs.mkShell {
      buildInputs = [ jls pkgs.valgrind ];
    };
  });
}
