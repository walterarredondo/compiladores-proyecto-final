#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "codegen.h"

static void gen_statement(ASTNode *node, CodeGenContext *ctx);
static char* gen_expression(ASTNode *expr, CodeGenContext *ctx);
static void gen_param_gets(ASTNode *param, CodeGenContext *ctx);
static const char* get_func_label(const char *name);
static const char* get_func_ret_var(const char *name);

void emit(CodeGenContext *ctx, const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(ctx->output, format, args);
    va_end(args);
    fprintf(ctx->output, "\n");
}

char* gen_temp_register(CodeGenContext *ctx) {
    static char buffer[32];
    snprintf(buffer, sizeof(buffer), "_t%d", ctx->next_temp++);
    return strdup(buffer);
}

char* gen_label(CodeGenContext *ctx) {
    static char buffer[32];
    snprintf(buffer, sizeof(buffer), "L%d", ctx->next_label++);
    return strdup(buffer);
}

static const char* get_func_label(const char *name) {
    static char buffer[128];
    snprintf(buffer, sizeof(buffer), "func_%s", name);
    return buffer;
}

static const char* get_func_ret_var(const char *name) {
    static char buffer[128];
    snprintf(buffer, sizeof(buffer), "ret_%s", name);
    return buffer;
}

static char* gen_expression(ASTNode *expr, CodeGenContext *ctx) {
    if (!expr) return NULL;

    char *result = NULL;

    switch (expr->type) {
        case NODE_INT_LITERAL: {
            result = gen_temp_register(ctx);
            emit(ctx, "VAR %s", result);
            emit(ctx, "ASSIGN %d %s", expr->data.int_value, result);
            break;
        }

        case NODE_FLOAT_LITERAL: {
            result = gen_temp_register(ctx);
            emit(ctx, "VAR %s", result);
            emit(ctx, "ASSIGN %f %s", expr->data.float_value, result);
            break;
        }

        case NODE_BOOL_LITERAL: {
            result = gen_temp_register(ctx);
            emit(ctx, "VAR %s", result);
            emit(ctx, "ASSIGN %d %s", expr->data.bool_value, result);
            break;
        }

        case NODE_STRING_LITERAL: {
            result = gen_temp_register(ctx);
            emit(ctx, "VAR %s", result);
            emit(ctx, "ASSIGN %s %s", expr->data.string_value, result);
            break;
        }

        case NODE_IDENTIFIER: {
            result = strdup(expr->data.identifier);
            break;
        }
        
        case NODE_ARRAY_ACCESS: {
            fprintf(stderr, "Error: acceso a arrays no soportado en esta versión del generador de código\n");
            exit(1);
        }
        
        case NODE_BINOP: {
            char *left = gen_expression(expr->data.binop.left, ctx);
            char *right = gen_expression(expr->data.binop.right, ctx);
            result = gen_temp_register(ctx);
            emit(ctx, "VAR %s", result);
            
            switch (expr->data.binop.op) {
                case OP_ADD:
                    emit(ctx, "ADD %s %s %s", left, right, result);
                    break;
                case OP_SUB:
                    emit(ctx, "SUB %s %s %s", left, right, result);
                    break;
                case OP_MUL:
                    emit(ctx, "MUL %s %s %s", left, right, result);
                    break;
                case OP_DIV:
                    emit(ctx, "DIV %s %s %s", left, right, result);
                    break;
                case OP_MOD:
                    emit(ctx, "MOD %s %s %s", left, right, result);
                    break;
                case OP_EQ:
                    emit(ctx, "EQ %s %s %s", left, right, result);
                    break;
                case OP_NE:
                    emit(ctx, "NEQ %s %s %s", left, right, result);
                    break;
                case OP_LT:
                    emit(ctx, "LT %s %s %s", left, right, result);
                    break;
                case OP_GT:
                    emit(ctx, "GT %s %s %s", left, right, result);
                    break;
                case OP_LE:
                    emit(ctx, "LTE %s %s %s", left, right, result);
                    break;
                case OP_GE:
                    emit(ctx, "GTE %s %s %s", left, right, result);
                    break;
                case OP_AND:
                    emit(ctx, "AND %s %s %s", left, right, result);
                    break;
                case OP_OR:
                    emit(ctx, "OR %s %s %s", left, right, result);
                    break;
            }
            break;
        }
        
        case NODE_UNOP: {
            char *operand = gen_expression(expr->data.unop.operand, ctx);
            result = gen_temp_register(ctx);
            emit(ctx, "VAR %s", result);
            switch (expr->data.unop.op) {
                case OP_NEG:
                    emit(ctx, "SUB 0 %s %s", operand, result);
                    break;
                case OP_NOT:
                    emit(ctx, "EQ %s 0 %s", operand, result);
                    break;
            }
            break;
        }
        
        case NODE_LENGTH: {
            fprintf(stderr, "Error: operador .length no soportado en esta versión del generador de código\n");
            exit(1);
        }
        
        case NODE_FUNCTION_CALL: {
            ASTNode *arg = expr->data.function_call.arguments;
            while (arg) {
                char *arg_val = gen_expression(arg->data.argument.expression, ctx);
                emit(ctx, "PARAM %s", arg_val);
                arg = arg->data.argument.next;
            }

            const char *label = get_func_label(expr->data.function_call.func_name);
            emit(ctx, "GOSUB %s", label);

            Symbol *sym = lookup_symbol(ctx->symtable, expr->data.function_call.func_name);
            if (sym && sym->is_function && sym->return_type != TYPE_VOID) {
                const char *ret_var = get_func_ret_var(expr->data.function_call.func_name);
                result = gen_temp_register(ctx);
                emit(ctx, "VAR %s", result);
                emit(ctx, "ASSIGN %s %s", ret_var, result);
            } else {
                result = NULL;
            }
            break;
        }
        
        default:
            break;
    }
    
    return result;
}

static void gen_statement(ASTNode *node, CodeGenContext *ctx) {
    if (!node) return;
    
    switch (node->type) {
        case NODE_STATEMENT_LIST:
            gen_statement(node->data.stmt_list.statement, ctx);
            gen_statement(node->data.stmt_list.next, ctx);
            break;
            
        case NODE_DECLARATION: {
            if (ctx->current_function) {
                emit(ctx, "VAR %s", node->data.declaration.var_name);
                if (node->data.declaration.init_value) {
                    char *value = gen_expression(node->data.declaration.init_value, ctx);
                    emit(ctx, "ASSIGN %s %s", value, node->data.declaration.var_name);
                }
            } else {
                if (node->data.declaration.init_value) {
                    fprintf(stderr, "Error: inicialización global no soportada en esta versión del generador de código\n");
                    exit(1);
                }
            }
            break;
        }
        
        case NODE_ARRAY_DECLARATION: {
            fprintf(stderr, "Error: declaración de arrays no soportada en esta versión del generador de código\n");
            exit(1);
        }
        
        case NODE_ASSIGNMENT: {
            char *value = gen_expression(node->data.assignment.value, ctx);
            emit(ctx, "ASSIGN %s %s", value, node->data.assignment.var_name);
            break;
        }
        
        case NODE_ARRAY_ASSIGNMENT: {
            fprintf(stderr, "Error: asignación a arrays no soportada en esta versión del generador de código\n");
            exit(1);
        }
        
        case NODE_IF: {
            char *cond = gen_expression(node->data.if_stmt.condition, ctx);
            char *else_label = gen_label(ctx);
            char *end_label = gen_label(ctx);

            if (node->data.if_stmt.else_branch) {
                emit(ctx, "IFFALSE %s GOTO %s", cond, else_label);
                gen_statement(node->data.if_stmt.then_branch, ctx);
                emit(ctx, "GOTO %s", end_label);
                emit(ctx, "LABEL %s", else_label);
                gen_statement(node->data.if_stmt.else_branch, ctx);
                emit(ctx, "LABEL %s", end_label);
            } else {
                emit(ctx, "IFFALSE %s GOTO %s", cond, end_label);
                gen_statement(node->data.if_stmt.then_branch, ctx);
                emit(ctx, "LABEL %s", end_label);
            }
            break;
        }
        
        case NODE_WHILE: {
            char *start_label = gen_label(ctx);
            char *end_label = gen_label(ctx);
            
            emit(ctx, "LABEL %s", start_label);
            char *cond = gen_expression(node->data.while_stmt.condition, ctx);
            emit(ctx, "IFFALSE %s GOTO %s", cond, end_label);
            gen_statement(node->data.while_stmt.body, ctx);
            emit(ctx, "GOTO %s", start_label);
            emit(ctx, "LABEL %s", end_label);
            break;
        }
        
        case NODE_FOR: {
            char *start_label = gen_label(ctx);
            char *end_label = gen_label(ctx);
            
            gen_statement(node->data.for_stmt.init, ctx);
            emit(ctx, "LABEL %s", start_label);
            char *cond = gen_expression(node->data.for_stmt.condition, ctx);
            emit(ctx, "IFFALSE %s GOTO %s", cond, end_label);
            gen_statement(node->data.for_stmt.body, ctx);
            gen_statement(node->data.for_stmt.increment, ctx);
            emit(ctx, "GOTO %s", start_label);
            emit(ctx, "LABEL %s", end_label);
            break;
        }
        
        case NODE_FUNCTION_DEF: {
            const char *func_name = node->data.function_def.func_name;
            const char *label = get_func_label(func_name);

            emit(ctx, "");
            emit(ctx, "LABEL %s", label);

            ASTNode *param = node->data.function_def.parameters;
            while (param) {
                emit(ctx, "VAR %s", param->data.parameter.param_name);
                param = param->data.parameter.next;
            }

            ctx->current_function = func_name;
            ctx->current_return_type = node->data.function_def.return_type;

            gen_param_gets(node->data.function_def.parameters, ctx);
            gen_statement(node->data.function_def.body, ctx);

            ctx->current_function = NULL;
            ctx->current_return_type = TYPE_VOID;
            break;
        }
        
        case NODE_PIXEL: {
            char *x = gen_expression(node->data.pixel.x, ctx);
            char *y = gen_expression(node->data.pixel.y, ctx);
            char *c = gen_expression(node->data.pixel.color, ctx);
            emit(ctx, "PIXEL %s %s %s", x, y, c);
            break;
        }
        
        case NODE_KEY: {
            if (node->data.key.key_code->type == NODE_INT_LITERAL) {
                int code = node->data.key.key_code->data.int_value;
                int mapped = code;
                switch (code) {
                    case 87: mapped = 4; break;  /* W */
                    case 83: mapped = 5; break;  /* S */
                    case 65: mapped = 6; break;  /* A */
                    case 68: mapped = 7; break;  /* D */
                    case 27: mapped = 8; break;  /* ESC */
                    case 32: mapped = 8; break;  /* Space */
                    default: mapped = code; break;
                }
                emit(ctx, "KEY %d %s", mapped, node->data.key.dest_var);
            } else {
                char *key_val = gen_expression(node->data.key.key_code, ctx);
                emit(ctx, "KEY %s %s", key_val, node->data.key.dest_var);
            }
            break;
        }
        
        case NODE_INPUT: {
            emit(ctx, "INPUT %s", node->data.input.input_var);
            break;
        }
        
        case NODE_PRINT: {
            char *value = gen_expression(node->data.print.expression, ctx);
            emit(ctx, "PRINT %s", value);
            break;
        }
        
        case NODE_RETURN: {
            if (node->data.return_stmt.return_value) {
                char *ret_val = gen_expression(node->data.return_stmt.return_value, ctx);
                if (ctx->current_function && ctx->current_return_type != TYPE_VOID) {
                    const char *ret_var = get_func_ret_var(ctx->current_function);
                    emit(ctx, "ASSIGN %s %s", ret_val, ret_var);
                }
            }
            emit(ctx, "RETURN");
            break;
        }

        case NODE_FUNCTION_CALL: {
            (void)gen_expression(node, ctx);
            break;
        }
        
        default:
            break;
    }
}

static void gen_param_gets(ASTNode *param, CodeGenContext *ctx) {
    if (!param) return;
    gen_param_gets(param->data.parameter.next, ctx);
    emit(ctx, "PARAM_GET %s", param->data.parameter.param_name);
}

void generate_code(ASTNode *root, FILE *output, SymbolTable *table) {
    CodeGenContext ctx;
    ctx.output = output;
    ctx.next_temp = 0;
    ctx.next_label = 0;
    ctx.var_offset = 0;
    ctx.symtable = table;
    ctx.current_function = NULL;
    ctx.current_return_type = TYPE_VOID;

    emit(&ctx, "; Código generado por el compilador FIS-25");
    emit(&ctx, "; Arquitectura: FIS-25");

    for (int i = 0; i < MAX_SYMBOLS; i++) {
        Symbol *sym = table->symbols[i];
        while (sym) {
            if (sym->is_function) {
                if (sym->return_type != TYPE_VOID) {
                    const char *ret_var = get_func_ret_var(sym->name);
                    emit(&ctx, "VAR %s", ret_var);
                }
            } else if (!sym->is_array) {
                emit(&ctx, "VAR %s", sym->name);
            }
            sym = sym->next;
        }
    }

    emit(&ctx, "");
    emit(&ctx, "GOSUB func_main");
    char *end_label = gen_label(&ctx);
    emit(&ctx, "LABEL %s", end_label);
    emit(&ctx, "GOTO %s", end_label);
    emit(&ctx, "");

    gen_statement(root, &ctx);
    
    emit(&ctx, "; Fin del programa");
}
