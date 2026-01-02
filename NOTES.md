```c
/* Declare a fixed buffer for user input of size 2048 */
static char input[2048];
int main()
{
    /* Read a line of user input of maximum size 2048 */
    fgets(input, 2048, stdin);
    printf("No you're a %s", input);

    if (!fgets(buffer, sizeof(buffer), stdin)) break;
```

### Summary Comparison Table

| Feature | `char **args` | `char *arg` |
|------|-------------|------------|
| Points to | A pointer (or an array of pointers) | A character (or an array of characters) |
| Typical Data | A list of multiple strings | A single string |
| First Index (`[0]`) | A string (`char *`) | A character (`char`) |
| Memory Visual | Address → [Address] → `['A','B','\0']` | Address → `['A','B','\0']` |


char *arg (A Single String)
-----------------------------
Variable        Memory Location         Data (The String)
[ arg ] ------> [ 0x100 ] ------------> 'H' 'e' 'l' 'l' 'o' '\0'


`char **args (An array of strings)`
-----------------------------------
Variable        Pointer Array           String Data
[ args ] ----> [ 0x500 ] ------------> [ 0x100 ] ----> 'C' 'a' 't' '\0'
               [ 0x508 ] ------------> [ 0x200 ] ----> 'D' 'o' 'g' '\0'
               [ 0x516 ] ------------> [ 0x300 ] ----> 'B' 'i' 'r' 'd' '\0'



# Memory leaks 
1. The "Allocation" Keywords
Any time you see these functions, the program is asking the operating system for a piece of "Heap" memory. You can think of these as "Check-out" slips from a library. 

    malloc(size): The most common. It reserves a block of memory of a specific size.
    calloc(n, size): Like malloc, but clears the memory (sets it to zero) first.
    realloc(ptr, size): Resizes a previously allocated block.
    strdup(string): A "hidden" allocation. It creates a new copy of a string and uses malloc behind the scenes. 

The Rule: Every single call to these must eventually have exactly one matching call to free(). 
2. What a "Leak" Looks Like
A leak occurs when you lose the "pointer" (the address) to that memory before you call free(). Once the pointer is gone, that memory is "lost" to the system until the program closes. 

    Overwriting Pointers: If you do ptr = malloc(10); and then immediately do ptr = some_other_value;, the first 10 bytes are leaked because you no longer know where they are to free them.
    Going Out of Scope: If you allocate memory inside a function but don't return it or free it, the local variable holding that address disappears when the function ends.
    Looping Leaks: If you have malloc inside a while loop but only free at the very end of the program, you are leaking memory every single time the loop runs. 

3. Structural Ownership (The Lisp Example)
In the Lisp parser provided, we used a tree structure. To know if it's correct, you check for "Nested Ownership":

    The Parent owns the Children: The SExp list node contains an array of pointers to other SExp nodes.
    Recursive Cleanup: To free the parent, you must first tell every child to free itself. If you just free(parent), you've leaked all the children. This is why we wrote a free_sexp function that calls itself. 





# version 2
``` c 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* 1. Data Structure Definitions */
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

/* 2. Lexer: Tokenization */
char* next_token() {
    while (*input_ptr && isspace(*input_ptr)) input_ptr++;
    if (*input_ptr == '\0') return NULL;

    const char *start = input_ptr;
    if (*input_ptr == '(' || *input_ptr == ')') {
        input_ptr++;
    } else {
        while (*input_ptr && !isspace(*input_ptr) && *input_ptr != '(' && *input_ptr != ')') {
            input_ptr++;
        }
    }
    
    int len = input_ptr - start;
    char *token = malloc(len + 1);
    strncpy(token, start, len);
    token[len] = '\0';
    return token;
}

/* 3. Parser: parse_sexp() and parse_list() */
SExp* parse_sexp(char *token);

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
    if (token) free(token); // Free the ")"
    return node;
}

SExp* parse_sexp(char *token) {
    if (strcmp(token, "(") == 0) {
        free(token);
        return parse_list();
    }

    SExp *node = malloc(sizeof(SExp));
    char *endptr;
    long val = strtol(token, &endptr, 10);

    if (*endptr == '\0') {
        node->type = TYPE_INT; 
        node->val.i = val; 
        free(token);
    } else {
        node->type = TYPE_SYM; 
        node->val.s = token;
    }
    return node;
}

/* 4. Memory Cleanup: Recursive free_sexp() */
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

/* 5. Helper: Print the tree */
void print_sexp(SExp *node) {
    if (node->type == TYPE_INT) printf("%ld ", node->val.i);
    else if (node->type == TYPE_SYM) printf("%s ", node->val.s);
    else if (node->type == TYPE_LIST) {
        printf("(");
        for (int i = 0; i < node->val.list.count; i++) {
            print_sexp(node->val.list.elements[i]);
        }
        printf(") ");
    }
}

int main() {
    input_ptr = "(+ 1 (* 2 3))";
    printf("Input: %s\n", input_ptr);

    char *first_token = next_token();
    if (first_token) {
        SExp *root = parse_sexp(first_token);
        printf("Parsed: ");
        print_sexp(root);
        printf("\n");

        free_sexp(root);
        printf("Memory cleaned up successfully.\n");
    }
    return 0;
}
```
