#ifndef SYMTABLE_H
#define SYMTABLE_H

#include "ast.h"

#define MAX_SYMBOLS 1000

typedef struct Symbol {
    char *name;
    DataType type;
    int is_array;
    int array_size;
    int is_function;
    DataType return_type;
    int address;  // Para generación de código
    struct Symbol *next;
} Symbol;

typedef struct SymbolTable {
    Symbol *symbols[MAX_SYMBOLS];
    int scope_level;
    struct SymbolTable *parent;
} SymbolTable;

// Funciones de la tabla de símbolos
SymbolTable* create_symbol_table();
SymbolTable* enter_scope(SymbolTable *current);
SymbolTable* exit_scope(SymbolTable *current);

Symbol* add_symbol(SymbolTable *table, char *name, DataType type);
Symbol* add_array_symbol(SymbolTable *table, char *name, DataType element_type, int size);
Symbol* add_function_symbol(SymbolTable *table, char *name, DataType return_type);

Symbol* lookup_symbol(SymbolTable *table, char *name);
Symbol* lookup_symbol_current_scope(SymbolTable *table, char *name);

void free_symbol_table(SymbolTable *table);

// Análisis semántico
void semantic_analysis(ASTNode *root, SymbolTable *table);
DataType check_expression_type(ASTNode *expr, SymbolTable *table);

#endif
