#include "comb.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

// Creates a new leaf node. The caller is responsible for freeing the
// returned node.
term_t *new_leaf(char c) {
    term_t *leaf = malloc(sizeof(term_t));
    leaf->c = c;
    leaf->is_leaf = 1;
    leaf->left = NULL;
    leaf->right = NULL;
    return leaf;
}

// Creates a new internal node. The caller is responsible for freeing the
// returned node.
term_t *new_node(term_t *left, term_t *right) {
    assert(left != NULL);
    term_t *node = malloc(sizeof(term_t));
    node->c = '\0';
    node->is_leaf = 0;
    node->left = left;
    node->right = right;
    return node;
}

// Frees the given term node and all of its children.
void free_term(term_t *term) {
    if (term == NULL) {
        return;
    }
    free_term(term->left);
    free_term(term->right);
    free(term);
}

// Prints the given term node and all of its children. The output is a
// string representation of the term, with left-associativity assumed and
// brackets used to override this. For example, 'Sa(bc)' is the same term as
// '((Sa)(bc))', with the latter including all of the implied brackets.
// The caller is responsible for freeing the returned string.
char *print_term(term_t *term) {
    assert(term != NULL);

    // If the term is a leaf, then it is an atomic term:
    if (term->is_leaf) {
        char *str = malloc(2);
        str[0] = term->c;
        str[1] = '\0';
        return str;
    }

    // Left child is never bracketed, but right child needs to be bracketed 
    // if it is longer than a single character, by left-associativity.
    char *left = print_term(term->left);
    char *right = term->right ? print_term(term->right) : NULL;
    int len = strlen(left) + (right ? strlen(right) : 0) + 1;
    if (right && strlen(right) > 1) {
        len += 2;
    }

    char *str = malloc(len);
    strcpy(str, left);
    if (right && strlen(right) > 1) {
        strcat(str, "(");
    }
    if (right) {
        strcat(str, right);
    }
    if (right && strlen(right) > 1) {
        strcat(str, ")");
    }

    free(left);
    free(right);
    return str;
}

// Just do dummy implementations of the rest for now:
term_t *parse_term(const char *str) {
    return NULL;
}

term_t *reduce_term(term_t *term) {
    return NULL;
}
