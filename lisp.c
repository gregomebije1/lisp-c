#define _POSIX_C_SOURCE 200809L /* Unlock POSIX featueres e.g strndup*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* If we are compiling on Windows compile these functions */
#ifdef _WIN32
#include <string.h>

static char buffer[2048];

/* Fake readline function */
char* readline(char* prompt) {
    fputs(prompt, stdout);
    fgets(buffer, 2048, stdin);
    char* cpy = malloc(strlen(buffer)+1);
    strcpy(cpy, buffer);
    cpy[strlen(cpy)-1] = '\0';
    return cpy;
}

/* Fake add_history function */
void add_history(char* unused) {}

/* Otherwise include the editline headers */
#else 
#include <readline/readline.h>
#include <readline/history.h>
#endif

typedef enum { TYPE_INT, TYPE_SYM, TYPE_LIST } Type; /* TYPE_INT = 0 ....*/

/* The Abstract Syntax Tree (AST) structure. It rep a single node in a tree
 * union { ... } val;: A union allows multiple variables to share the 
 * same memory location. Only one member of the union can be used at a 
 *  time to save memory.

    long i: Used if the node is an integer.
    char *s: Used if the node is a symbol (like a variable or function name).
    struct { ... } list: Used if the node is a list. It contains a pointer 
    to an array of other SExp pointers (elements) and a count of how many
    items are in that list.
 */

typedef struct SExp {
    Type type;
    union {
        long i;
        char *s;
        struct { struct SExp **elements; int count; } list;
    } val;
} SExp;

// --- Memory Management ---
void free_sexp(SExp *node) {
    if (!node) return;
    if (node->type == TYPE_SYM) {
        free(node->val.s);
    } else if (node->type == TYPE_LIST) {
        for (int i = 0; i < node->val.list.count; i++) {
            free_sexp(node->val.list.elements[i]);
        }
        free(node->val.list.elements);
    }
    free(node);
}

SExp* parse_sexp(const char **input);

/* `const char **input` points to the actual xter not the array of chars*/
void skip_ws(const char **input) {
    while (**input && isspace(**input)) (*input)++;
}

SExp* parse_atom(const char **input) {
    const char *start = *input; /* Marks where the word starts */
    /* Keep looping until we find a space or bracket */
    while (**input && !isspace(**input) && **input != '(' && **input != ')') (*input)++;
    int len = *input - start; /*How long was that word? */

    /*char *token = strndup(start, len); copy the word into its own memory*/

    /* Manual version of strndup: 
     * Request enough memory (+1 for the null-terminator)
     */
    char *token = malloc(len + 1); 
    memcpy(token, start, len);      /* Copy the characters */
    token[len] = '\0';             /* Manually add the end-of-string marker */

    SExp *node = malloc(sizeof(SExp)); /*Allocte mem(heap) for SExp structure*/
    char *endptr;
    
    /* Integer conversion
     * Attempts to convert the string token into a base-10 long integer. 
     * The endptr tracks where the conversion stopped C Library strtol.
     */
    long val = strtol(token, &endptr, 10);
    if (*endptr == '\0') { /* Conversion reached the end of string (NULL)*/
        node->type = TYPE_INT; node->val.i = val; free(token);
    } else {
        node->type = TYPE_SYM; node->val.s = token;
    }
    return node;
}

SExp* parse_list(const char **input) {
    (*input)++; // skip '('
    SExp *node = malloc(sizeof(SExp)); /*Allocates mem for a new node */
    node->type = TYPE_LIST; /* mark the new node as a list */

    /* Setup the dynamic array*/
    node->val.list.elements = NULL;
    node->val.list.count = 0;

    while (1) {
        skip_ws(input);
        if (**input == ')' || **input == '\0') { /*end if we see ')'*/
            if (**input == ')') (*input)++;
            break;
        }
        SExp *child = parse_sexp(input); /* RECURSSION: Parse what is inside*/
        node->val.list.count++;

	/*Every time a new child is parsed, the array of pointers is grown by one slot using realloc to accommodate the new child.
	 */
        node->val.list.elements = realloc(node->val.list.elements, sizeof(SExp*) * node->val.list.count);

	/*The newly created child node is stored in the array, and the list's count is incremented.
	*/ 
        node->val.list.elements[node->val.list.count - 1] = child;
    }
    return node;
}

SExp* parse_sexp(const char **input) {
    skip_ws(input);
    if (**input == '(') return parse_list(input);
    return parse_atom(input);
}

// --- Evaluation Logic ---
SExp* eval(SExp *node) {
    if (!node) return NULL;

    // Numbers evaluate to themselves
    if (node->type == TYPE_INT) return node;

    // Evaluate Lists
    if (node->type == TYPE_LIST && node->val.list.count > 0) {
        SExp *op = node->val.list.elements[0];
        
        // Simple "+" implementation
        if (op->type == TYPE_SYM && strcmp(op->val.s, "+") == 0) {
            long sum = 0;
            for (int i = 1; i < node->val.list.count; i++) {
                SExp *res = eval(node->val.list.elements[i]);
                if (res->type == TYPE_INT) sum += res->val.i;
            }
            SExp *res_node = malloc(sizeof(SExp));
            res_node->type = TYPE_INT;
            res_node->val.i = sum;
            return res_node;
        }
    }
    
    // Default: return a "copy" or itself (caution: simplified for this example)
    return node;
}

void print_sexp(SExp *node) {
    if (!node) return;
    if (node->type == TYPE_INT) printf("%ld", node->val.i);
    else if (node->type == TYPE_SYM) printf("%s", node->val.s);
    else if (node->type == TYPE_LIST) {
        printf("(");
        for (int i = 0; i < node->val.list.count; i++) {
            print_sexp(node->val.list.elements[i]);
            if (i < node->val.list.count - 1) printf(" ");
        }
        printf(")");
    }
}
int main(int argc, char** argv) {
    printf("Lyper 1.0.0.1\n");
    printf("Press Ctrl+C or type 'exit' to quit.\n");


    char* input;  /*Pointer to inputed text*/
    while ((input = readline("hyper> ")) != NULL) {

        /* 1. Add non-empty input to history for up/down arrow retrieval*/
        if (strlen(input) > 0) {
            add_history(input);
        }

        /* 2. Check for exit command */
        if (strcmp(input, "exit") == 0) {
            free(input);
            break;
        }

        /* 3. READ 
	 * Ceate temp pointer to "walk" through the text without loosing the
	 * start of the string 
	 * */
        const char *ptr = input; 
        SExp *ast = parse_sexp(&ptr);
        
        /* 4. EVAL & PRINT */
        if (ast) {
            SExp *result = eval(ast);
            printf("=> ");
            print_sexp(result);
            printf("\n");
            
            // Cleanup
            if (result != ast) free_sexp(result);
            free_sexp(ast);
        }

        /* 5. readline uses malloc, so you must free the input string*/
        free(input);
    }
}
