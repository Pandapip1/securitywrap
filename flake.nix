{
  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs =
    {
      self,
      nixpkgs,
      flake-utils,
    }:
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
        lib = pkgs.lib;
        stdenv = pkgs.stdenv;
      in
      {
        lib.mkSecurityWrapper =
          {
            executable,
            setUID ? null,
            resetUID ? false,
            setRealUID ? null,
            setGID ? null,
            resetGID ? false,
            setRealGID ? null,
          }:
          stdenv.mkDerivation {
            name = "securitywrap-${builtins.baseNameOf executable}";
            src = ./.;
            nativeBuildInputs = [ pkgs.cmake ];
            cmakeFlags = [
              (lib.cmakeFeature "WRAP_EXECUTABLE" executable)
              (lib.optionalString (setUID != null) (lib.cmakeFeature "SET_UID" (builtins.toString setUID)))
              (lib.optionalString resetUID (lib.cmakeBool "RESET_UID" true))
              (lib.optionalString (setRealUID != null) (
                lib.cmakeFeature "SET_REAL_UID" (builtins.toString setRealUID)
              ))
              (lib.optionalString (setGID != null) (lib.cmakeFeature "SET_GID" (builtins.toString setGID)))
              (lib.optionalString resetGID (lib.cmakeBool "RESET_GID" true))
              (lib.optionalString (setRealGID != null) (
                lib.cmakeFeature "SET_REAL_GID" (builtins.toString setRealGID)
              ))
            ];
            meta = {
              description = "A simple tool to run a command with real and/or effective uids and/or gids";
              license = lib.licenses.lgpl3;
              maintainers = with lib.maintainers; [ pandapip1 ];
              mainProgram = builtins.baseNameOf executable;
            };
          };
      }
    );
}
