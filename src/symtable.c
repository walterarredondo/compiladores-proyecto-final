#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtable.h"

static unsigned int hash(char *str) {
    unsigned int hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;
    return hash % MAX_SYMBOLS;
}

SymbolTable* create_symbol_table() {
    SymbolTable *table = (SymbolTable*)malloc(sizeof(SymbolTable));
    if (!table) {
        fprintf(stderr, "Error: No se pudo asignar memoria para la tabla de símbolos\n");
        exit(1);
    }
    
    for (int i = 0; i < MAX_SYMBOLS; i++) {
        table->symbols[i] = NULL;
    }
    table->scope_level = 0;
    table->parent = NULL;
    
    return table;
}

SymbolTable* enter_scope(SymbolTable *current) {
    SymbolTable *new_table = create_symbol_table();
    new_table->scope_level = current->scope_level + 1;
    new_table->parent = current;
    return new_table;
}

SymbolTable* exit_scope(SymbolTable *current) {
    SymbolTable *parent = current->parent;
    // No liberamos la tabla aquí para mantener los símbolos durante la generación de código
    return parent;
}

Symbol* add_symbol(SymbolTable *table, char *name, DataType type) {
    unsigned int index = hash(name);
    
    // Verificar si ya existe en el scope actual
    Symbol *existing = lookup_symbol_current_scope(table, name);
    if (existing) {
        fprintf(stderr, "Error semántico: Variable '%s' ya declarada en este ámbito\n", name);
        exit(1);
    }
    
    Symbol *symbol = (Symbol*)malloc(sizeof(Symbol));
    symbol->name = strdup(name);
    symbol->type = type;
    symbol->is_array = 0;
    symbol->is_function = 0;
    symbol->address = 0;
    symbol->next = table->symbols[index];
    table->symbols[index] = symbol;
    
    return symbol;
}

Symbol* add_array_symbol(SymbolTable *table, char *name, DataType element_type, int size) {
    Symbol *symbol = add_symbol(table, name, element_type);
    symbol->is_array = 1;
    symbol->array_size = size;
    return symbol;
}

Symbol* add_function_symbol(SymbolTable *table, char *name, DataType return_type) {
    Symbol *symbol = add_symbol(table, name, return_type);
    symbol->is_function = 1;
    symbol->return_type = return_type;
    return symbol;
}

Symbol* lookup_symbol_current_scope(SymbolTable *table, char *name) {
    unsigned int index = hash(name);
    Symbol *symbol = table->symbols[index];
    
    while (symbol) {
        if (strcmp(symbol->name, name) == 0) {
            return symbol;
        }
        symbol = symbol->next;
    }
    
    return NULL;
}

Symbol* lookup_symbol(SymbolTable *table, char *name) {
    SymbolTable *current = table;
    
    while (current) {
        Symbol *symbol = lookup_symbol_current_scope(current, name);
        if (symbol) {
            return symbol;
        }
        current = current->parent;
    }
    
    return NULL;
}

void free_symbol_table(SymbolTable *table) {
    if (!table) return;
    
    for (int i = 0; i < MAX_SYMBOLS; i++) {
        Symbol *symbol = table->symbols[i];
        while (symbol) {
            Symbol *next = symbol->next;
            free(symbol->name);
            free(symbol);
            symbol = next;
        }
    }
    
    free(table);
}

// Análisis semántico
static void analyze_statement(ASTNode *node, SymbolTable *table);

DataType check_expression_type(ASTNode *expr, SymbolTable *table) {
    if (!expr) return TYPE_VOID;
    
    switch (expr->type) {
        case NODE_INT_LITERAL:
            return TYPE_INT;
        case NODE_FLOAT_LITERAL:
            return TYPE_FLOAT;
        case NODE_BOOL_LITERAL:
            return TYPE_BOOL;
        case NODE_STRING_LITERAL:
            return TYPE_STRING;
            
        case NODE_IDENTIFIER: {
            Symbol *sym = lookup_symbol(table, expr->data.identifier);
            if (!sym) {
                fprintf(stderr, "Error semántico: Variable '%s' no declarada\n", 
                        expr->data.identifier);
                exit(1);
            }
            return sym->type;
        }
        
        case NODE_ARRAY_ACCESS: {
            Symbol *sym = lookup_symbol(table, expr->data.array_access.array_name);
            if (!sym) {
                fprintf(stderr, "Error semántico: Array '%s' no declarado\n", 
                        expr->data.array_access.array_name);
                exit(1);
            }
            if (!sym->is_array) {
                fprintf(stderr, "Error semántico: '%s' no es un array\n", 
                        expr->data.array_access.array_name);
                exit(1);
            }
            DataType index_type = check_expression_type(expr->data.array_access.index, table);
            if (index_type != TYPE_INT) {
                fprintf(stderr, "Error semántico: Índice de array debe ser entero\n");
                exit(1);
            }
            return sym->type;
        }
        
        case NODE_BINOP: {
            DataType left_type = check_expression_type(expr->data.binop.left, table);
            DataType right_type = check_expression_type(expr->data.binop.right, table);

            switch (expr->data.binop.op) {
                case OP_ADD:
                case OP_SUB:
                case OP_MUL:
                case OP_DIV:
                case OP_MOD:
                    if (!((left_type == TYPE_INT || left_type == TYPE_FLOAT) &&
                          (right_type == TYPE_INT || right_type == TYPE_FLOAT))) {
                        fprintf(stderr, "Error semántico: Operaciones aritméticas solo permitidas entre int y float\n");
                        exit(1);
                    }
                    return (left_type == TYPE_FLOAT || right_type == TYPE_FLOAT) ? 
                           TYPE_FLOAT : TYPE_INT;
                case OP_EQ:
                case OP_NE:
                    if (left_type != right_type &&
                        !((left_type == TYPE_INT && right_type == TYPE_FLOAT) ||
                          (left_type == TYPE_FLOAT && right_type == TYPE_INT))) {
                        fprintf(stderr, "Error semántico: Comparación de igualdad entre tipos incompatibles\n");
                        exit(1);
                    }
                    return TYPE_BOOL;
                case OP_LT:
                case OP_GT:
                case OP_LE:
                case OP_GE:
                    if (!((left_type == TYPE_INT || left_type == TYPE_FLOAT) &&
                          (right_type == TYPE_INT || right_type == TYPE_FLOAT))) {
                        fprintf(stderr, "Error semántico: Comparaciones relacionales solo permitidas entre int y float\n");
                        exit(1);
                    }
                    return TYPE_BOOL;
                case OP_AND:
                case OP_OR:
                    if (left_type != TYPE_BOOL || right_type != TYPE_BOOL) {
                        fprintf(stderr, "Error semántico: Operadores lógicos requieren operandos booleanos\n");
                        exit(1);
                    }
                    return TYPE_BOOL;
                default:
                    return TYPE_INT;
            }
        }
        
        case NODE_UNOP:
            return check_expression_type(expr->data.unop.operand, table);
        
        case NODE_LENGTH:
            return TYPE_INT;
            
        case NODE_FUNCTION_CALL: {
            Symbol *sym = lookup_symbol(table, expr->data.function_call.func_name);
            if (!sym) {
                fprintf(stderr, "Error semántico: Función '%s' no declarada\n", 
                        expr->data.function_call.func_name);
                exit(1);
            }
            return sym->return_type;
        }
        
        default:
            return TYPE_VOID;
    }
}

static void analyze_statement(ASTNode *node, SymbolTable *table) {
    if (!node) return;
    
    switch (node->type) {
        case NODE_STATEMENT_LIST:
            analyze_statement(node->data.stmt_list.statement, table);
            analyze_statement(node->data.stmt_list.next, table);
            break;
            
        case NODE_DECLARATION: {
            add_symbol(table, 
                      node->data.declaration.var_name, 
                      node->data.declaration.var_type);
            if (node->data.declaration.init_value) {
                DataType init_type = check_expression_type(
                    node->data.declaration.init_value, table);
                if (init_type != node->data.declaration.var_type && 
                    !(init_type == TYPE_INT && node->data.declaration.var_type == TYPE_FLOAT)) {
                    fprintf(stderr, "Error semántico: Tipo incompatible en inicialización\n");
                    exit(1);
                }
            }
            break;
        }
        
        case NODE_ARRAY_DECLARATION: {
            int size = 0;
            ASTNode *elem = node->data.array_decl.elements;
            while (elem) {
                size++;
                elem = elem->data.argument.next;
            }
            add_array_symbol(table, 
                           node->data.array_decl.array_name, 
                           node->data.array_decl.element_type,
                           size);
            break;
        }
        
        case NODE_ASSIGNMENT: {
            Symbol *sym = lookup_symbol(table, node->data.assignment.var_name);
            if (!sym) {
                fprintf(stderr, "Error semántico: Variable '%s' no declarada\n", 
                        node->data.assignment.var_name);
                exit(1);
            }
            DataType value_type = check_expression_type(node->data.assignment.value, table);
            if (value_type != sym->type && 
                !(value_type == TYPE_INT && sym->type == TYPE_FLOAT)) {
                fprintf(stderr, "Error semántico: Tipo incompatible en asignación\n");
                exit(1);
            }
            break;
        }
        
        case NODE_IF:
            if (check_expression_type(node->data.if_stmt.condition, table) != TYPE_BOOL) {
                fprintf(stderr, "Error semántico: La condición del if debe ser de tipo bool\n");
                exit(1);
            }
            analyze_statement(node->data.if_stmt.then_branch, table);
            analyze_statement(node->data.if_stmt.else_branch, table);
            break;
            
        case NODE_WHILE:
            if (check_expression_type(node->data.while_stmt.condition, table) != TYPE_BOOL) {
                fprintf(stderr, "Error semántico: La condición del while debe ser de tipo bool\n");
                exit(1);
            }
            analyze_statement(node->data.while_stmt.body, table);
            break;
            
        case NODE_FOR:
            analyze_statement(node->data.for_stmt.init, table);
            if (check_expression_type(node->data.for_stmt.condition, table) != TYPE_BOOL) {
                fprintf(stderr, "Error semántico: La condición del for debe ser de tipo bool\n");
                exit(1);
            }
            analyze_statement(node->data.for_stmt.increment, table);
            analyze_statement(node->data.for_stmt.body, table);
            break;
            
        case NODE_FUNCTION_DEF: {
            add_function_symbol(table, 
                              node->data.function_def.func_name,
                              node->data.function_def.return_type);
            SymbolTable *func_scope = enter_scope(table);
            
            // Añadir parámetros al scope de la función
            ASTNode *param = node->data.function_def.parameters;
            while (param) {
                add_symbol(func_scope, 
                          param->data.parameter.param_name,
                          param->data.parameter.param_type);
                param = param->data.parameter.next;
            }
            
            analyze_statement(node->data.function_def.body, func_scope);
            break;
        }
        
        case NODE_PIXEL:
            check_expression_type(node->data.pixel.x, table);
            check_expression_type(node->data.pixel.y, table);
            check_expression_type(node->data.pixel.color, table);
            break;
            
        case NODE_PRINT:
            check_expression_type(node->data.print.expression, table);
            break;
            
        case NODE_RETURN:
            check_expression_type(node->data.return_stmt.return_value, table);
            break;
            
        default:
            break;
    }
}

void semantic_analysis(ASTNode *root, SymbolTable *table) {
    analyze_statement(root, table);
}
