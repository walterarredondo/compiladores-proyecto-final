%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "symtable.h"
#include "codegen.h"

extern int yylex();
extern int yylineno;
extern FILE *yyin;

void yyerror(const char *s);

ASTNode *root = NULL;
SymbolTable *global_symtable;
%}

%union {
    int ival;
    float fval;
    int bval;
    char *sval;
    struct ASTNode *node;
    int type;
}

%token <ival> INT_LITERAL
%token <fval> FLOAT_LITERAL
%token <sval> STRING_LITERAL
%token <sval> IDENTIFIER
%token <bval> TRUE FALSE

%token INT FLOAT BOOL STRING
%token IF ELSE WHILE FOR RETURN FUNC
%token PIXEL KEY INPUT PRINT LENGTH
%token EQ NE LE GE AND OR ARROW

%type <node> program statement_list statement
%type <node> declaration assignment
%type <node> if_statement while_statement for_statement
%type <node> function_def parameter_list
%type <node> expression term factor
%type <node> array_declaration array_access
%type <node> function_call argument_list
%type <node> pixel_stmt key_stmt input_stmt print_stmt
%type <type> type

%left OR
%left AND
%left EQ NE
%left '<' '>' LE GE
%left '+' '-'
%left '*' '/' '%'
%right '!' UMINUS

%%

program:
    statement_list { root = $1; }
    ;

statement_list:
    statement { $$ = $1; }
    | statement_list statement { 
        $$ = create_statement_list($1, $2); 
      }
    ;

statement:
    declaration ';' { $$ = $1; }
    | assignment ';' { $$ = $1; }
    | if_statement { $$ = $1; }
    | while_statement { $$ = $1; }
    | for_statement { $$ = $1; }
    | function_def { $$ = $1; }
    | pixel_stmt ';' { $$ = $1; }
    | key_stmt ';' { $$ = $1; }
    | input_stmt ';' { $$ = $1; }
    | print_stmt ';' { $$ = $1; }
    | RETURN expression ';' { $$ = create_return_node($2); }
    | function_call ';' { $$ = $1; }
    | '{' statement_list '}' { $$ = $2; }
    ;

type:
    INT { $$ = TYPE_INT; }
    | FLOAT { $$ = TYPE_FLOAT; }
    | BOOL { $$ = TYPE_BOOL; }
    | STRING { $$ = TYPE_STRING; }
    ;

declaration:
    type IDENTIFIER { 
        $$ = create_declaration_node($1, $2, NULL); 
    }
    | type IDENTIFIER '=' expression { 
        $$ = create_declaration_node($1, $2, $4); 
    }
    | type '[' ']' IDENTIFIER '=' '[' argument_list ']' {
        $$ = create_array_declaration_node($1, $4, $7);
    }
    ;

assignment:
    IDENTIFIER '=' expression { 
        $$ = create_assignment_node($1, $3); 
    }
    | array_access '=' expression {
        $$ = create_array_assignment_node($1, $3);
    }
    ;

array_declaration:
    type '[' ']' IDENTIFIER { 
        $$ = create_array_declaration_node($1, $4, NULL); 
    }
    ;

array_access:
    IDENTIFIER '[' expression ']' { 
        $$ = create_array_access_node($1, $3); 
    }
    ;

if_statement:
    IF '(' expression ')' statement { 
        $$ = create_if_node($3, $5, NULL); 
    }
    | IF '(' expression ')' statement ELSE statement { 
        $$ = create_if_node($3, $5, $7); 
    }
    ;

while_statement:
    WHILE '(' expression ')' statement { 
        $$ = create_while_node($3, $5); 
    }
    ;

for_statement:
    FOR '(' assignment ';' expression ';' assignment ')' statement {
        $$ = create_for_node($3, $5, $7, $9);
    }
    ;

function_def:
    FUNC IDENTIFIER '(' parameter_list ')' ARROW type '{' statement_list '}' {
        $$ = create_function_node($2, $4, $7, $9);
    }
    | FUNC IDENTIFIER '(' ')' ARROW type '{' statement_list '}' {
        $$ = create_function_node($2, NULL, $6, $8);
    }
    ;

parameter_list:
    type IDENTIFIER {
        $$ = create_parameter_node($1, $2, NULL);
    }
    | parameter_list ',' type IDENTIFIER {
        $$ = create_parameter_node($3, $4, $1);
    }
    ;

pixel_stmt:
    PIXEL '(' expression ',' expression ',' expression ')' {
        $$ = create_pixel_node($3, $5, $7);
    }
    ;

key_stmt:
    KEY '(' expression ',' IDENTIFIER ')' {
        $$ = create_key_node($3, $5);
    }
    ;

input_stmt:
    INPUT '(' IDENTIFIER ')' {
        $$ = create_input_node($3);
    }
    ;

print_stmt:
    PRINT '(' expression ')' {
        $$ = create_print_node($3);
    }
    ;

function_call:
    IDENTIFIER '(' argument_list ')' {
        $$ = create_function_call_node($1, $3);
    }
    | IDENTIFIER '(' ')' {
        $$ = create_function_call_node($1, NULL);
    }
    ;

argument_list:
    expression {
        $$ = create_argument_node($1, NULL);
    }
    | argument_list ',' expression {
        $$ = create_argument_node($3, $1);
    }
    ;

expression:
    term { $$ = $1; }
    | expression '+' expression { $$ = create_binop_node(OP_ADD, $1, $3); }
    | expression '-' expression { $$ = create_binop_node(OP_SUB, $1, $3); }
    | expression '*' expression { $$ = create_binop_node(OP_MUL, $1, $3); }
    | expression '/' expression { $$ = create_binop_node(OP_DIV, $1, $3); }
    | expression '%' expression { $$ = create_binop_node(OP_MOD, $1, $3); }
    | expression EQ expression { $$ = create_binop_node(OP_EQ, $1, $3); }
    | expression NE expression { $$ = create_binop_node(OP_NE, $1, $3); }
    | expression '<' expression { $$ = create_binop_node(OP_LT, $1, $3); }
    | expression '>' expression { $$ = create_binop_node(OP_GT, $1, $3); }
    | expression LE expression { $$ = create_binop_node(OP_LE, $1, $3); }
    | expression GE expression { $$ = create_binop_node(OP_GE, $1, $3); }
    | expression AND expression { $$ = create_binop_node(OP_AND, $1, $3); }
    | expression OR expression { $$ = create_binop_node(OP_OR, $1, $3); }
    | '-' expression %prec UMINUS { $$ = create_unop_node(OP_NEG, $2); }
    | '!' expression { $$ = create_unop_node(OP_NOT, $2); }
    | array_access '.' LENGTH { $$ = create_length_node($1); }
    ;

term:
    factor { $$ = $1; }
    | '(' expression ')' { $$ = $2; }
    ;

factor:
    INT_LITERAL { $$ = create_int_literal_node($1); }
    | FLOAT_LITERAL { $$ = create_float_literal_node($1); }
    | STRING_LITERAL { $$ = create_string_literal_node($1); }
    | TRUE { $$ = create_bool_literal_node(1); }
    | FALSE { $$ = create_bool_literal_node(0); }
    | IDENTIFIER { $$ = create_identifier_node($1); }
    | array_access { $$ = $1; }
    | function_call { $$ = $1; }
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Error de sintaxis en línea %d: %s\n", yylineno, s);
    exit(1);
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <archivo_entrada.src> <archivo_salida.asm>\n", argv[0]);
        return 1;
    }

    yyin = fopen(argv[1], "r");
    if (!yyin) {
        fprintf(stderr, "Error: No se puede abrir el archivo %s\n", argv[1]);
        return 1;
    }

    global_symtable = create_symbol_table();

    printf("=== Compilando %s ===\n", argv[1]);
    
    if (yyparse() == 0) {
        printf("✓ Análisis sintáctico completado\n");
        
        // Análisis semántico
        printf("✓ Verificando semántica...\n");
        semantic_analysis(root, global_symtable);
        printf("✓ Análisis semántico completado\n");
        
        // Generación de código
        printf("✓ Generando código FIS-25...\n");
        FILE *output = fopen(argv[2], "w");
        if (!output) {
            fprintf(stderr, "Error: No se puede crear el archivo %s\n", argv[2]);
            return 1;
        }
        
        generate_code(root, output, global_symtable);
        fclose(output);
        
        printf("✓ Código generado exitosamente en %s\n", argv[2]);
        printf("=== Compilación exitosa ===\n");
    } else {
        fprintf(stderr, "✗ Error en la compilación\n");
        return 1;
    }

    fclose(yyin);
    free_symbol_table(global_symtable);
    free_ast(root);
    
    return 0;
}
