#ifndef AST_H
#define AST_H

// Tipos de datos
typedef enum {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_BOOL,
    TYPE_STRING,
    TYPE_ARRAY,
    TYPE_VOID
} DataType;

// Tipos de nodos
typedef enum {
    NODE_PROGRAM,
    NODE_STATEMENT_LIST,
    NODE_DECLARATION,
    NODE_ASSIGNMENT,
    NODE_ARRAY_DECLARATION,
    NODE_ARRAY_ACCESS,
    NODE_ARRAY_ASSIGNMENT,
    NODE_IF,
    NODE_WHILE,
    NODE_FOR,
    NODE_FUNCTION_DEF,
    NODE_FUNCTION_CALL,
    NODE_PARAMETER,
    NODE_ARGUMENT,
    NODE_RETURN,
    NODE_BINOP,
    NODE_UNOP,
    NODE_INT_LITERAL,
    NODE_FLOAT_LITERAL,
    NODE_STRING_LITERAL,
    NODE_BOOL_LITERAL,
    NODE_IDENTIFIER,
    NODE_PIXEL,
    NODE_KEY,
    NODE_INPUT,
    NODE_PRINT,
    NODE_LENGTH
} NodeType;

// Operadores binarios
typedef enum {
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_MOD,
    OP_EQ,
    OP_NE,
    OP_LT,
    OP_GT,
    OP_LE,
    OP_GE,
    OP_AND,
    OP_OR
} BinaryOperator;

// Operadores unarios
typedef enum {
    OP_NEG,
    OP_NOT
} UnaryOperator;

// Estructura del nodo AST
typedef struct ASTNode {
    NodeType type;
    DataType data_type;
    
    union {
        // Literales
        int int_value;
        float float_value;
        int bool_value;
        char *string_value;
        
        // Identificador
        char *identifier;
        
        // Operadores
        struct {
            BinaryOperator op;
            struct ASTNode *left;
            struct ASTNode *right;
        } binop;
        
        struct {
            UnaryOperator op;
            struct ASTNode *operand;
        } unop;
        
        // Declaración
        struct {
            DataType var_type;
            char *var_name;
            struct ASTNode *init_value;
        } declaration;
        
        // Asignación
        struct {
            char *var_name;
            struct ASTNode *value;
        } assignment;
        
        // Array
        struct {
            DataType element_type;
            char *array_name;
            struct ASTNode *elements;
        } array_decl;
        
        struct {
            char *array_name;
            struct ASTNode *index;
        } array_access;
        
        struct {
            struct ASTNode *array_access;
            struct ASTNode *value;
        } array_assign;
        
        // Control de flujo
        struct {
            struct ASTNode *condition;
            struct ASTNode *then_branch;
            struct ASTNode *else_branch;
        } if_stmt;
        
        struct {
            struct ASTNode *condition;
            struct ASTNode *body;
        } while_stmt;
        
        struct {
            struct ASTNode *init;
            struct ASTNode *condition;
            struct ASTNode *increment;
            struct ASTNode *body;
        } for_stmt;
        
        // Funciones
        struct {
            char *func_name;
            struct ASTNode *parameters;
            DataType return_type;
            struct ASTNode *body;
        } function_def;
        
        struct {
            char *func_name;
            struct ASTNode *arguments;
        } function_call;
        
        struct {
            DataType param_type;
            char *param_name;
            struct ASTNode *next;
        } parameter;
        
        struct {
            struct ASTNode *expression;
            struct ASTNode *next;
        } argument;
        
        // Return
        struct {
            struct ASTNode *return_value;
        } return_stmt;
        
        // Hardware FIS-25
        struct {
            struct ASTNode *x;
            struct ASTNode *y;
            struct ASTNode *color;
        } pixel;
        
        struct {
            struct ASTNode *key_code;
            char *dest_var;
        } key;
        
        struct {
            char *input_var;
        } input;
        
        struct {
            struct ASTNode *expression;
        } print;
        
        struct {
            struct ASTNode *array;
        } length;
        
        // Lista de sentencias
        struct {
            struct ASTNode *statement;
            struct ASTNode *next;
        } stmt_list;
    } data;
} ASTNode;

// Funciones para crear nodos
ASTNode* create_int_literal_node(int value);
ASTNode* create_float_literal_node(float value);
ASTNode* create_string_literal_node(char *value);
ASTNode* create_bool_literal_node(int value);
ASTNode* create_identifier_node(char *name);

ASTNode* create_binop_node(BinaryOperator op, ASTNode *left, ASTNode *right);
ASTNode* create_unop_node(UnaryOperator op, ASTNode *operand);

ASTNode* create_declaration_node(DataType type, char *name, ASTNode *init_value);
ASTNode* create_assignment_node(char *name, ASTNode *value);

ASTNode* create_array_declaration_node(DataType type, char *name, ASTNode *elements);
ASTNode* create_array_access_node(char *name, ASTNode *index);
ASTNode* create_array_assignment_node(ASTNode *array_access, ASTNode *value);

ASTNode* create_if_node(ASTNode *condition, ASTNode *then_branch, ASTNode *else_branch);
ASTNode* create_while_node(ASTNode *condition, ASTNode *body);
ASTNode* create_for_node(ASTNode *init, ASTNode *condition, ASTNode *increment, ASTNode *body);

ASTNode* create_function_node(char *name, ASTNode *parameters, DataType return_type, ASTNode *body);
ASTNode* create_function_call_node(char *name, ASTNode *arguments);
ASTNode* create_parameter_node(DataType type, char *name, ASTNode *next);
ASTNode* create_argument_node(ASTNode *expression, ASTNode *next);

ASTNode* create_return_node(ASTNode *value);

ASTNode* create_pixel_node(ASTNode *x, ASTNode *y, ASTNode *color);
ASTNode* create_key_node(ASTNode *key_code, char *dest_var);
ASTNode* create_input_node(char *var_name);
ASTNode* create_print_node(ASTNode *expression);
ASTNode* create_length_node(ASTNode *array);

ASTNode* create_statement_list(ASTNode *stmt1, ASTNode *stmt2);

void free_ast(ASTNode *node);

#endif
