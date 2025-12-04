%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.tab.h"

extern int yylineno;
%}

%option noyywrap

%%

[ \t]+              { /* Ignorar espacios y tabs */ }
\n                  { yylineno++; }
"//".*              { /* Comentarios de línea */ }

"int"               { return INT; }
"float"             { return FLOAT; }
"bool"              { return BOOL; }
"string"            { return STRING; }
"true"              { yylval.bval = 1; return TRUE; }
"false"             { yylval.bval = 0; return FALSE; }

"if"                { return IF; }
"else"              { return ELSE; }
"while"             { return WHILE; }
"for"               { return FOR; }
"return"            { return RETURN; }
"func"              { return FUNC; }

"pixel"             { return PIXEL; }
"key"               { return KEY; }
"input"             { return INPUT; }
"print"             { return PRINT; }

"length"            { return LENGTH; }

[0-9]+              { yylval.ival = atoi(yytext); return INT_LITERAL; }
[0-9]+\.[0-9]+      { yylval.fval = atof(yytext); return FLOAT_LITERAL; }
\"([^\\\"]|\\.)*\"  { 
                      yylval.sval = strdup(yytext); 
                      return STRING_LITERAL; 
                    }

[a-zA-Z_][a-zA-Z0-9_]*  { 
                          yylval.sval = strdup(yytext); 
                          return IDENTIFIER; 
                        }

"=="                { return EQ; }
"!="                { return NE; }
"<="                { return LE; }
">="                { return GE; }
"&&"                { return AND; }
"||"                { return OR; }
"->"                { return ARROW; }

"+"                 { return '+'; }
"-"                 { return '-'; }
"*"                 { return '*'; }
"/"                 { return '/'; }
"%"                 { return '%'; }
"="                 { return '='; }
"<"                 { return '<'; }
">"                 { return '>'; }
"!"                 { return '!'; }
"("                 { return '('; }
")"                 { return ')'; }
"{"                 { return '{'; }
"}"                 { return '}'; }
"["                 { return '['; }
"]"                 { return ']'; }
","                 { return ','; }
";"                 { return ';'; }
":"                 { return ':'; }
"."                 { return '.'; }

.                   { 
                      fprintf(stderr, "Error léxico en línea %d: caracter inválido '%s'\n", 
                              yylineno, yytext); 
                      exit(1);
                    }

%%
