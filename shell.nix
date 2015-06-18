let pkgs = import <nixpkgs> {}; in
with pkgs;
stdenv.mkDerivation rec {
  name = "metro";

  src = ./.;

  buildInputs = [
    libnfc
  ];
}
