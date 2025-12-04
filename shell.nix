# Compatibilidad con nix-shell para usuarios que no usan flakes
# Uso: nix-shell

{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
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
  ];
  
  shellHook = ''
    echo "     Compilador FIS-25 - Entorno de Desarrollo"
    echo "     Triángulo de Sierpinski"
    echo ""
    echo "Comandos rápidos:"
    echo "   make all       → Compilar el compilador (binario en build/)"
    echo "   make example   → Compilar example/sierpinski.src → build/sierpinski.asm"
    echo "   make test      → Compilar y probar"
    echo "   make clean     → Limpiar archivos generados"
    echo ""
    echo "Tip: Considera usar 'nix develop' si tienes flakes habilitado"
    echo ""
  '';
}
