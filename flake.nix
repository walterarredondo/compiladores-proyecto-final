{
  description = "Compilador FIS-25 - Triángulo de Sierpinski";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { inherit system; };
        
        # Definir el paquete del compilador
        compiler-fis25 = pkgs.stdenv.mkDerivation {
          pname = "compiler-fis25";
          version = "1.0.0";
          
          src = ./.;
          
          nativeBuildInputs = with pkgs; [
            flex
            bison
            gnumake
          ];
          
          buildInputs = with pkgs; [
            gcc
          ];
          
          buildPhase = ''
            make all
            make example
          '';
          
          installPhase = ''
            mkdir -p $out/bin
            mkdir -p $out/share/compiler-fis25/examples
            mkdir -p $out/share/doc/compiler-fis25
            
            # Instalar el compilador
            cp build/compiler $out/bin/compiler-fis25
            
            # Instalar ejemplos
            cp example/sierpinski.src $out/share/compiler-fis25/examples/
            cp build/sierpinski.asm $out/share/compiler-fis25/examples/
            
          '';
          
          meta = with pkgs.lib; {
            description = "Compilador para arquitectura FIS-25 - Triángulo de Sierpinski";
            homepage = "https://github.com/walterarredondo/compiler-fis25";
            license = licenses.mit;
            platforms = platforms.unix;
          };
        };
        
      in
      {
        # Paquete por defecto
        packages.default = compiler-fis25;
        packages.compiler-fis25 = compiler-fis25;
        
        # DevShell con todas las herramientas de desarrollo
        devShells.default = pkgs.mkShell {
          name = "compiler-fis25-dev";
          
          buildInputs = with pkgs; [
            # Herramientas de compilación
            gcc
            gnumake
            flex
            bison
            
            # Herramientas de desarrollo
            gdb
            valgrind
            
            # Utilidades
            git
            tree
            which
            
            # Editor (opcional)
            # neovim
            # vscode
          ];
          
        };
        
        # Aplicaciones que se pueden ejecutar directamente
        apps.default = {
          type = "app";
          program = "${compiler-fis25}/bin/compiler-fis25";
        };
        
        # Checks para CI/CD
        checks = {
          build = compiler-fis25;
          
          # Test de compilación del ejemplo
          test-example = pkgs.stdenv.mkDerivation {
            name = "test-sierpinski";
            src = ./.;
            
            nativeBuildInputs = with pkgs; [
              flex
              bison
              gnumake
            ];
            
            buildInputs = [ pkgs.gcc ];
            
            buildPhase = ''
              make example
            '';
            
            installPhase = ''
              mkdir -p $out
              cp build/sierpinski.asm $out/
              echo "Test passed: sierpinski.asm generated successfully" > $out/test-result.txt
            '';
          };
        };
      });
}
