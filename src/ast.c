#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

static ASTNode* create_node(NodeType type) {
    ASTNode *node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "Error: No se pudo asignar memoria para el nodo AST\n");
        exit(1);
    }
    node->type = type;
    node->data_type = TYPE_VOID;
    return node;
}

ASTNode* create_int_literal_node(int value) {
    ASTNode *node = create_node(NODE_INT_LITERAL);
    node->data_type = TYPE_INT;
    node->data.int_value = value;
    return node;
}

ASTNode* create_float_literal_node(float value) {
    ASTNode *node = create_node(NODE_FLOAT_LITERAL);
    node->data_type = TYPE_FLOAT;
    node->data.float_value = value;
    return node;
}

ASTNode* create_string_literal_node(char *value) {
    ASTNode *node = create_node(NODE_STRING_LITERAL);
    node->data_type = TYPE_STRING;
    node->data.string_value = strdup(value);
    return node;
}

ASTNode* create_bool_literal_node(int value) {
    ASTNode *node = create_node(NODE_BOOL_LITERAL);
    node->data_type = TYPE_BOOL;
    node->data.bool_value = value;
    return node;
}

ASTNode* create_identifier_node(char *name) {
    ASTNode *node = create_node(NODE_IDENTIFIER);
    node->data.identifier = strdup(name);
    return node;
}

ASTNode* create_binop_node(BinaryOperator op, ASTNode *left, ASTNode *right) {
    ASTNode *node = create_node(NODE_BINOP);
    node->data.binop.op = op;
    node->data.binop.left = left;
    node->data.binop.right = right;
    return node;
}

ASTNode* create_unop_node(UnaryOperator op, ASTNode *operand) {
    ASTNode *node = create_node(NODE_UNOP);
    node->data.unop.op = op;
    node->data.unop.operand = operand;
    return node;
}

ASTNode* create_declaration_node(DataType type, char *name, ASTNode *init_value) {
    ASTNode *node = create_node(NODE_DECLARATION);
    node->data.declaration.var_type = type;
    node->data.declaration.var_name = strdup(name);
    node->data.declaration.init_value = init_value;
    return node;
}

ASTNode* create_assignment_node(char *name, ASTNode *value) {
    ASTNode *node = create_node(NODE_ASSIGNMENT);
    node->data.assignment.var_name = strdup(name);
    node->data.assignment.value = value;
    return node;
}

ASTNode* create_array_declaration_node(DataType type, char *name, ASTNode *elements) {
    ASTNode *node = create_node(NODE_ARRAY_DECLARATION);
    node->data_type = TYPE_ARRAY;
    node->data.array_decl.element_type = type;
    node->data.array_decl.array_name = strdup(name);
    node->data.array_decl.elements = elements;
    return node;
}

ASTNode* create_array_access_node(char *name, ASTNode *index) {
    ASTNode *node = create_node(NODE_ARRAY_ACCESS);
    node->data.array_access.array_name = strdup(name);
    node->data.array_access.index = index;
    return node;
}

ASTNode* create_array_assignment_node(ASTNode *array_access, ASTNode *value) {
    ASTNode *node = create_node(NODE_ARRAY_ASSIGNMENT);
    node->data.array_assign.array_access = array_access;
    node->data.array_assign.value = value;
    return node;
}

ASTNode* create_if_node(ASTNode *condition, ASTNode *then_branch, ASTNode *else_branch) {
    ASTNode *node = create_node(NODE_IF);
    node->data.if_stmt.condition = condition;
    node->data.if_stmt.then_branch = then_branch;
    node->data.if_stmt.else_branch = else_branch;
    return node;
}

ASTNode* create_while_node(ASTNode *condition, ASTNode *body) {
    ASTNode *node = create_node(NODE_WHILE);
    node->data.while_stmt.condition = condition;
    node->data.while_stmt.body = body;
    return node;
}

ASTNode* create_for_node(ASTNode *init, ASTNode *condition, ASTNode *increment, ASTNode *body) {
    ASTNode *node = create_node(NODE_FOR);
    node->data.for_stmt.init = init;
    node->data.for_stmt.condition = condition;
    node->data.for_stmt.increment = increment;
    node->data.for_stmt.body = body;
    return node;
}

ASTNode* create_function_node(char *name, ASTNode *parameters, DataType return_type, ASTNode *body) {
    ASTNode *node = create_node(NODE_FUNCTION_DEF);
    node->data.function_def.func_name = strdup(name);
    node->data.function_def.parameters = parameters;
    node->data.function_def.return_type = return_type;
    node->data.function_def.body = body;
    return node;
}

ASTNode* create_function_call_node(char *name, ASTNode *arguments) {
    ASTNode *node = create_node(NODE_FUNCTION_CALL);
    node->data.function_call.func_name = strdup(name);
    node->data.function_call.arguments = arguments;
    return node;
}

ASTNode* create_parameter_node(DataType type, char *name, ASTNode *next) {
    ASTNode *node = create_node(NODE_PARAMETER);
    node->data.parameter.param_type = type;
    node->data.parameter.param_name = strdup(name);
    node->data.parameter.next = next;
    return node;
}

ASTNode* create_argument_node(ASTNode *expression, ASTNode *next) {
    ASTNode *node = create_node(NODE_ARGUMENT);
    node->data.argument.expression = expression;
    node->data.argument.next = next;
    return node;
}

ASTNode* create_return_node(ASTNode *value) {
    ASTNode *node = create_node(NODE_RETURN);
    node->data.return_stmt.return_value = value;
    return node;
}

ASTNode* create_pixel_node(ASTNode *x, ASTNode *y, ASTNode *color) {
    ASTNode *node = create_node(NODE_PIXEL);
    node->data.pixel.x = x;
    node->data.pixel.y = y;
    node->data.pixel.color = color;
    return node;
}

ASTNode* create_key_node(ASTNode *key_code, char *dest_var) {
    ASTNode *node = create_node(NODE_KEY);
    node->data.key.key_code = key_code;
    node->data.key.dest_var = strdup(dest_var);
    return node;
}

ASTNode* create_input_node(char *var_name) {
    ASTNode *node = create_node(NODE_INPUT);
    node->data.input.input_var = strdup(var_name);
    return node;
}

ASTNode* create_print_node(ASTNode *expression) {
    ASTNode *node = create_node(NODE_PRINT);
    node->data.print.expression = expression;
    return node;
}

ASTNode* create_length_node(ASTNode *array) {
    ASTNode *node = create_node(NODE_LENGTH);
    node->data_type = TYPE_INT;
    node->data.length.array = array;
    return node;
}

ASTNode* create_statement_list(ASTNode *stmt1, ASTNode *stmt2) {
    ASTNode *node = create_node(NODE_STATEMENT_LIST);
    node->data.stmt_list.statement = stmt1;
    node->data.stmt_list.next = stmt2;
    return node;
}

void free_ast(ASTNode *node) {
    if (!node) return;
    
    switch (node->type) {
        case NODE_BINOP:
            free_ast(node->data.binop.left);
            free_ast(node->data.binop.right);
            break;
        case NODE_UNOP:
            free_ast(node->data.unop.operand);
            break;
        case NODE_DECLARATION:
            free(node->data.declaration.var_name);
            free_ast(node->data.declaration.init_value);
            break;
        case NODE_ASSIGNMENT:
            free(node->data.assignment.var_name);
            free_ast(node->data.assignment.value);
            break;
        case NODE_STATEMENT_LIST:
            free_ast(node->data.stmt_list.statement);
            free_ast(node->data.stmt_list.next);
            break;
        // ... otros casos según sea necesario
        default:
            break;
    }
    
    free(node);
}
