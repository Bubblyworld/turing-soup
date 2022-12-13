#pragma once

/* Terms are represented as strings over the alphabet 'SKIBCW()abc...z', and
 * are required to have balanced parentheses. Uppercase characters are
 * the combinators, and lowercase characters are variables. Terms are 
 * evaluated as left-associative, with brackets to override this, meaning 
 * that a term like 'Sa(bc)' is equivalent to '((Sa)(bc))'.
 *
 * The supported combinators, along with their derivation schemas, are:
 *   Sxyz -> xz(yz)
 *   Kxy  -> x
 *   Ix   -> x
 *   Bxyz -> x(yz)
 *   Cxyz -> xzy
 *   Wxy  -> xyy
 *
 * The internal representation of terms is a tree of nodes, where each node
 * is either a combinator or a variable. The tree is constructed by parsing
 * the string representation of the term, and then evaluated by traversing
 * the tree left-to-right and applying the appropriate combinators.
 */

typedef struct term {
		char c; // '\0' unless is_leaf
	  int is_leaf; // 0 if internal node, 1 if leaf
		struct term *left; // NULL if leaf, not NULL otherwise
		struct term *right; // NULL if leaf, not NULL otherwise
} term_t;

term_t *new_leaf(char c);
term_t *new_node(term_t *left, term_t *right);
term_t *copy_term(term_t *term);
void free_term(term_t *term);
char *print_term(term_t *term);
term_t *parse_term(const char *str);
term_t *reduce_term(term_t *term);

// We need to be able to free strings returned by print_term:
void free(void *ptr);
