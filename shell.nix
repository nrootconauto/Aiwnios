{
	pkgs ? import <nixpkgs> { }
}:

pkgs.mkShell {
	buildInputs = with pkgs; [
		ninja
		SDL2
		cmake
	];
}

