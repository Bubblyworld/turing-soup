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
    assert(right != NULL);
    term_t *node = malloc(sizeof(term_t));
    node->c = '\0';
    node->is_leaf = 0;
    node->left = left;
    node->right = right;
    return node;
}

// Copies the given term. The caller is responsible for freeing the
// returned term.
term_t *copy_term(term_t *term) {
    if (term->is_leaf) {
        return new_leaf(term->c);
    } else {
        return new_node(copy_term(term->left), copy_term(term->right));
    }
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

int validate(const char *str) {
    assert(str != NULL);

    // The str must have balanced parentheses, and never close a bracket
    // before opening one:
    int count = 0;
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] == '(') {
            count++;
        } else if (str[i] == ')') {
            count--;
            if (count < 0) {
                return 0;
            }
        }
        if (count < 0) {
            return 0;
        }
    }
    if (count != 0) {
        return 0;
    }

    // The str must only contain the allowed characters:
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] != 'S' && str[i] != 'K' && str[i] != 'I' &&
            str[i] != 'B' && str[i] != 'C' && str[i] != 'W' &&
            str[i] != '(' && str[i] != ')' && str[i] != 'a' &&
            str[i] != 'b' && str[i] != 'c' && str[i] != 'd' &&
            str[i] != 'e' && str[i] != 'f' && str[i] != 'g' &&
            str[i] != 'h' && str[i] != 'i' && str[i] != 'j' &&
            str[i] != 'k' && str[i] != 'l' && str[i] != 'm' &&
            str[i] != 'n' && str[i] != 'o' && str[i] != 'p' &&
            str[i] != 'q' && str[i] != 'r' && str[i] != 's' &&
            str[i] != 't' && str[i] != 'u' && str[i] != 'v' &&
            str[i] != 'w' && str[i] != 'x' && str[i] != 'y' &&
            str[i] != 'z') {
            return 0;
        }
    }

    return 1;
}

// Parses the given string into a term node. The caller is responsible for
// freeing the returned node. Returns NULL if the string is invalid.
term_t *parse_term(const char *str) {
    assert(validate(str));

    // If the string is empty, then it is an invalid term:
    if (strlen(str) == 0) {
        return NULL;
    }

    // We walk the string, parsing the terms left-to-right. If we encounter
    // a bracket, then we parse the contents of the brackets recursively.
    term_t *term = NULL;
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] == '(') {
            // Find the matching closing bracket:
            int count = 1;
            int j = i + 1;
            while (count > 0) {
                if (str[j] == '(') {
                    count++;
                } else if (str[j] == ')') {
                    count--;
                }
                j++;
            }

            // Parse the contents of the brackets:
            char *sub = malloc(j - i);
            strncpy(sub, str + i + 1, j - i - 2);
            sub[j - i - 2] = '\0';
            term_t *sub_term = parse_term(sub);
            free(sub);

            // If the term is empty, then the brackets were invalid:
            if (sub_term == NULL) {
                free_term(term);
                return NULL;
            }

            // Append the parsed term to the current term:
            if (term == NULL) {
                term = sub_term;
            } else {
                term_t *node = new_node(term, sub_term);
                term = node;
            }

            // Skip the contents of the brackets:
            i = j - 1;
        } else {
            // Create a new leaf node for the current character:
            term_t *leaf = new_leaf(str[i]);

            // Append the leaf to the current term:
            if (term == NULL) {
                term = leaf;
            } else {
                term_t *node = new_node(term, leaf);
                term = node;
            }
        }
    }

    return term;
}

// Reduces every redex in the given term by a single step, starting from the
// right-most redex. This function does not mutate the given term, but instead
// returns a new term. The caller is responsible for freeing the returned term.
term_t *reduce_term(term_t *term) {
    assert(term != NULL);

    // We walk the tree down to the left-most leaf using a stack to keep track
    // of the right-hand children.
    term_t **stack = malloc(sizeof(term_t *));
    term_t *node = term;
    int stack_size = 0;
    while (node != NULL) {
        if (node->is_leaf) {
            break;
        }

        stack = realloc(stack, (stack_size + 1) * sizeof(term_t *));
        stack[stack_size] = reduce_term(node->right);
        stack_size++;
        node = node->left;
    }

    // If the leaf-node is not a combinator, then we simply reconstruct the
    // tree from the stack and return it, as we cannot reduce anything:
    if (node->c != 'S' && node->c != 'K' && node->c != 'I' &&
        node->c != 'B' && node->c != 'C' && node->c != 'W') {
        term_t *result = copy_term(node);
        for (int i = stack_size - 1; i >= 0; i--) {
            result = new_node(result, stack[i]);
        }
        free(stack);
        return result;
    }

    // If the leaf-node is a combinator, then we can reduce it, assuming there
    // are enough arguments to apply the corresponding derivation rule:
    //   Sxyz -> xz(yz)
    //   Kxy  -> x
    //   Ix   -> x
    //   Bxyz -> x(yz)
    //   Cxyz -> xzy
    //   Wxy  -> xyy
    term_t *result;
    if (node->c == 'S' && stack_size >= 3) {
        result = new_node(
            new_node(
                stack[stack_size - 1],
                stack[stack_size - 3]
            ),
            new_node(
                stack[stack_size - 2],
                copy_term(stack[stack_size - 3])
            )
        );
        stack_size -= 3;
    } else if (node->c == 'K' && stack_size >= 2) {
        result = stack[stack_size - 1];
        stack_size -= 2;
    } else if (node->c == 'I' && stack_size >= 1) {
        result = stack[stack_size - 1];
        stack_size -= 1;
    } else if (node->c == 'B' && stack_size >= 3) {
        result = new_node(
            stack[stack_size - 1],
            new_node(
                stack[stack_size - 2],
                stack[stack_size - 3]
            )
        );
        stack_size -= 3;
    } else if (node->c == 'C' && stack_size >= 3) {
        result = new_node(
            new_node(
                stack[stack_size - 1],
                stack[stack_size - 3]
            ),
            stack[stack_size - 2]
        );
        stack_size -= 3;
    } else if (node->c == 'W' && stack_size >= 2) {
        result = new_node(
            new_node(
                stack[stack_size - 1],
                stack[stack_size - 2]
            ),
            copy_term(stack[stack_size - 2])
        );
        stack_size -= 2;
    } else {
        result = copy_term(node);
    }

    // Finally, we reconstruct the tree from the remaining stack and return it:
    for (int i = stack_size - 1; i >= 0; i--) {
        result = new_node(result, stack[i]);
    }
    free(stack);
    return result;
}
