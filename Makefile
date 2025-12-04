# Makefile para el compilador FIS-25
# Proyecto: Triángulo de Sierpinski

CC = gcc
CFLAGS = -Wall -Wextra -g
LEX = flex
YACC = bison

SRCDIR = src
BUILDDIR = build
EXAMPLEDIR = example

# Archivos fuente y objetos
SRC_OBJECTS = \
	$(BUILDDIR)/ast.o \
	$(BUILDDIR)/symtable.o \
	$(BUILDDIR)/codegen.o

GEN_OBJECTS = \
	$(BUILDDIR)/parser.tab.o \
	$(BUILDDIR)/lex.yy.o

OBJECTS = $(SRC_OBJECTS) $(GEN_OBJECTS)

# Ejecutable del compilador (en build/)
COMPILER = $(BUILDDIR)/compiler

# Programa de ejemplo
EXAMPLE_SRC = $(EXAMPLEDIR)/sierpinski.src
EXAMPLE_ASM = $(BUILDDIR)/sierpinski.asm

.PHONY: all clean distclean test example help

all: $(COMPILER)

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

# Generar el compilador
$(COMPILER): $(OBJECTS) | $(BUILDDIR)
	$(CC) $(CFLAGS) -o $(COMPILER) $(OBJECTS) -lfl

# Generar el parser con Bison
$(BUILDDIR)/parser.tab.c $(BUILDDIR)/parser.tab.h: $(SRCDIR)/parser.y | $(BUILDDIR)
	$(YACC) -d -o $(BUILDDIR)/parser.tab.c $(SRCDIR)/parser.y

# Generar el lexer con Flex
$(BUILDDIR)/lex.yy.c: $(SRCDIR)/lexer.l $(BUILDDIR)/parser.tab.h | $(BUILDDIR)
	$(LEX) -o $(BUILDDIR)/lex.yy.c $(SRCDIR)/lexer.l

# Compilar archivos objeto generados
$(BUILDDIR)/parser.tab.o: $(BUILDDIR)/parser.tab.c $(SRCDIR)/ast.h $(SRCDIR)/symtable.h $(SRCDIR)/codegen.h | $(BUILDDIR)
	$(CC) $(CFLAGS) -I$(SRCDIR) -I$(BUILDDIR) -c $< -o $@

$(BUILDDIR)/lex.yy.o: $(BUILDDIR)/lex.yy.c $(BUILDDIR)/parser.tab.h | $(BUILDDIR)
	$(CC) $(CFLAGS) -I$(SRCDIR) -I$(BUILDDIR) -c $< -o $@

# Compilar archivos objeto de src/
$(BUILDDIR)/ast.o: $(SRCDIR)/ast.c $(SRCDIR)/ast.h | $(BUILDDIR)
	$(CC) $(CFLAGS) -I$(SRCDIR) -I$(BUILDDIR) -c $< -o $@

$(BUILDDIR)/symtable.o: $(SRCDIR)/symtable.c $(SRCDIR)/symtable.h $(SRCDIR)/ast.h | $(BUILDDIR)
	$(CC) $(CFLAGS) -I$(SRCDIR) -I$(BUILDDIR) -c $< -o $@

$(BUILDDIR)/codegen.o: $(SRCDIR)/codegen.c $(SRCDIR)/codegen.h $(SRCDIR)/ast.h $(SRCDIR)/symtable.h | $(BUILDDIR)
	$(CC) $(CFLAGS) -I$(SRCDIR) -I$(BUILDDIR) -c $< -o $@

# Compilar el programa de ejemplo
example: $(COMPILER)
	@echo "=== Compilando el programa de ejemplo: $(EXAMPLE_SRC) ==="
	./$(COMPILER) $(EXAMPLE_SRC) $(EXAMPLE_ASM)
	@echo ""
	@echo "Código generado en: $(EXAMPLE_ASM)"
	@echo "Puedes ejecutarlo en el simulador FIS-25"

# Prueba rápida
test: example
	@echo ""
	@echo "=== Mostrando las primeras líneas del código generado ==="
	@head -n 30 $(EXAMPLE_ASM)
	@echo "..."
	@echo ""
	@echo "Ver archivo completo: cat $(EXAMPLE_ASM)"

# Limpiar archivos generados
clean:
	rm -rf $(BUILDDIR)/*


# Ayuda
help:
	@echo "Makefile del compilador FIS-25"
	@echo ""
	@echo "Objetivos disponibles:"
	@echo "  make all       - Compila el compilador (binario en build/)"
	@echo "  make example   - Compila el programa de ejemplo (Sierpinski)"
	@echo "  make test      - Compila y muestra parte del código generado"
	@echo "  make clean     - Elimina archivos generados en build/"
	@echo "  make help      - Muestra esta ayuda"
	@echo ""
	@echo "Uso del compilador:"
	@echo "  ./build/compiler <archivo_entrada.src> <archivo_salida.asm>"
	@echo ""
	@echo "Ejemplo:"
	@echo "  ./build/compiler example/sierpinski.src build/sierpinski.asm"
