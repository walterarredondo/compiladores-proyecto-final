#ifndef CODEGEN_H
#define CODEGEN_H

#include <stdio.h>
#include "ast.h"
#include "symtable.h"

// Contexto de generación de código
typedef struct CodeGenContext {
    FILE *output;
    int next_temp;      // Siguiente variable temporal
    int next_label;     // Siguiente etiqueta
    int var_offset;     // Reservado (no usado actualmente)
    SymbolTable *symtable;
    const char *current_function;
    DataType current_return_type;
} CodeGenContext;

// Funciones principales
void generate_code(ASTNode *root, FILE *output, SymbolTable *table);

// Funciones auxiliares
char* gen_temp_register(CodeGenContext *ctx);
char* gen_label(CodeGenContext *ctx);
void emit(CodeGenContext *ctx, const char *format, ...);

#endif
