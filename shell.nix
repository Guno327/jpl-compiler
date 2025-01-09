{ pkgs ? import <nixpkgs> {}}: let
  inherit (pkgs) lib stdenv;
in
pkgs.mkShell {
  NIX_LD_LIBRARY_PATH = lib.makeLibraryPath [
    stdenv.cc.cc
    pkgs.mlib
    pkgs.zlib
    pkgs.nasm
  ];
  NIX_LD = lib.fileContents "${stdenv.cc}/nix-support/dynamic-linker";

  packages = with pkgs; [
    gnumake
    clang
    
    nasm
    zlib
    mlib
  ];
}
