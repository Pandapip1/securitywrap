{
  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
        lib = pkgs.lib;
        stdenv = pkgs.stdenv;
      in
      {
        packages = rec {
          securitywrap = stdenv.mkDerivation {
            name = "securitywrap";
            src = ./.;
            nativeBuildInputs = [ pkgs.cmake ];
            meta = {
              description = "A simple tool to run a command with real and/or effective uids and/or gids";
              license = lib.licenses.lgpl3;
              maintainers = with lib.maintainers; [ pandapip1 ];
              mainProgram = "securitywrap";
            };
          };
          default = securitywrap;
        };
      }
    );
}
