#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <readline/readline.h>
#include <readline/history.h>

/* Data Structure Definitions */
typedef enum { TYPE_INT, TYPE_SYM, TYPE_LIST } Type;

typedef struct SExp {
    Type type;
    union {
        long i;
        char *s;
        struct { struct SExp **elements; int count; } list;
    } val;
} SExp;

const char *input_ptr;

/* Function Prototypes */
SExp* eval(SExp *node);
void free_sexp(SExp *node);
SExp* parse_sexp(char *token);

/* Lexer: Tokenization */
char* next_token() {
    while (*input_ptr && isspace((unsigned char)*input_ptr)) input_ptr++;
    if (*input_ptr == '\0') return NULL;

    const char *start = input_ptr;
    if (*input_ptr == '(' || *input_ptr == ')') {
        input_ptr++;
    } else {
        while (*input_ptr && !isspace((unsigned char)*input_ptr) && *input_ptr != '(' && *input_ptr != ')') {
            input_ptr++;
        }
    }
    
    int len = input_ptr - start;
    char *token = malloc(len + 1);
    strncpy(token, start, len);
    token[len] = '\0';
    return token;
}

/* Parser */
SExp* parse_list() {
    SExp *node = malloc(sizeof(SExp));
    node->type = TYPE_LIST;
    node->val.list.elements = NULL;
    node->val.list.count = 0;

    char *token;
    while ((token = next_token()) != NULL && strcmp(token, ")") != 0) {
        SExp *child = parse_sexp(token);
        node->val.list.elements = realloc(node->val.list.elements, 
                                          sizeof(SExp*) * (node->val.list.count + 1));
        node->val.list.elements[node->val.list.count++] = child;
    }
    if (token) free(token); // Free ")"
    return node;
}

SExp* parse_sexp(char *token) {
    if (!token) return NULL;
    if (strcmp(token, "(") == 0) {
        free(token);
        return parse_list();
    }

    SExp *node = malloc(sizeof(SExp));
    char *endptr;
    long val = strtol(token, &endptr, 10);

    if (*endptr == '\0' && strlen(token) > 0) {
        node->type = TYPE_INT; 
        node->val.i = val; 
        free(token);
    } else {
        node->type = TYPE_SYM; 
        node->val.s = token;
    }
    return node;
}

/* Evaluator */
SExp* eval(SExp *node) {
    if (!node) return NULL;
    if (node->type == TYPE_INT || node->type == TYPE_SYM) return node;

    if (node->type == TYPE_LIST && node->val.list.count > 0) {
        SExp *op = node->val.list.elements[0];
        if (op->type == TYPE_SYM) {
            if (strcmp(op->val.s, "+") == 0) {
                long sum = 0;
                for (int i = 1; i < node->val.list.count; i++) {
                    SExp *res = eval(node->val.list.elements[i]);
                    if (res->type == TYPE_INT) sum += res->val.i;
                    if (res != node->val.list.elements[i]) free_sexp(res);
                }
                SExp *new_node = malloc(sizeof(SExp));
                new_node->type = TYPE_INT; new_node->val.i = sum;
                return new_node;
            } else if (strcmp(op->val.s, "*") == 0) {
                long prod = 1;
                for (int i = 1; i < node->val.list.count; i++) {
                    SExp *res = eval(node->val.list.elements[i]);
                    if (res->type == TYPE_INT) prod *= res->val.i;
                    if (res != node->val.list.elements[i]) free_sexp(res);
                }
                SExp *new_node = malloc(sizeof(SExp));
                new_node->type = TYPE_INT; new_node->val.i = prod;
                return new_node;
            }
        }
    }
    return node;
}

/* Helper Functions */
void free_sexp(SExp *node) {
    if (!node) return;
    if (node->type == TYPE_SYM) free(node->val.s);
    else if (node->type == TYPE_LIST) {
        for (int i = 0; i < node->val.list.count; i++) free_sexp(node->val.list.elements[i]);
        free(node->val.list.elements);
    }
    free(node);
}

void print_sexp(SExp *node) {
    if (!node) return;
    if (node->type == TYPE_INT) printf("%ld ", node->val.i);
    else if (node->type == TYPE_SYM) printf("%s ", node->val.s);
    else if (node->type == TYPE_LIST) {
        printf("(");
        for (int i = 0; i < node->val.list.count; i++) print_sexp(node->val.list.elements[i]);
        printf(") ");
    }
}

int main() {
    printf("Lyper 1.0.0.1 (2026 Build)\n");
    char* input;
    while ((input = readline("hyper> ")) != NULL) {
        if (strlen(input) > 0) {
            add_history(input);
            if (strcmp(input, "exit") == 0) { free(input); break; }
            input_ptr = input;
            char *token = next_token();
            if (token) {
                SExp *ast = parse_sexp(token);
                SExp *result = eval(ast);
                printf("=> "); print_sexp(result); printf("\n");
                if (result != ast) free_sexp(result);
                free_sexp(ast);
            }
        }
        free(input);
    }
    return 0;
}
