#include "soup.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Converts a string literal into a (dynamically allocated) term. The caller
// is responsible for freeing the term.
term_t term_new(const char *str) {
	return strdup(str);
}

// Initialises a subterms_t.
void subterms_init(subterms_t *subterms) {
	subterms->size = 0;
}

// Removes any unnecessary brackets in the given term.
// TODO: there's some kind of bug here, IS((SI)S) -> IS(SI)S instead of IS(SIS)
void term_normalise(term_t term) {
	int i = 0; // writing index
	int count = 0; // bracket level
	int first_char = 1; // 0 if first char in new bracket level, 1 otherwise
	for (int j = 0; j < strlen(term); j++) {
		if (term[j] == '(') {
			if (first_char) {
				count++;
				continue;
			} else {
				// If the bracket contains a single character, we skip brackets:
				if (term[j+2] == ')') {
					term[i] = term[j+1];
					i++;
					j += 2;
					first_char = 0;
					continue;
				}

				term[i] = term[j];
				first_char = 1;
				count++;
				i++;
				continue;
			}
		} else if (term[j] == ')') {
			if (count == 0) {
				continue;
			} else {
				term[i] = term[j];
				count--;
				i++;
				continue;
			}
		}

		// We need to write this char:
		term[i] = term[j];
		first_char = 0;
		i++;
	}
	term[i]= '\0';
}

// Pushes an index to the end of a subterms_t.
void subterms_push(subterms_t *subterms, int index) {
	if (subterms->size >= 6) {
		subterms->indices[5] = index;
	} else {
		subterms->indices[subterms->size] = index;
		subterms->size++;
	}
}

// Copies a subterm from a term to another string.
void subterms_copy(subterms_t *subterms, term_t dest, term_t src, int index) {
	int start = subterms->indices[index];
	int end = subterms->indices[index + 1];
	strncpy(dest, src + start, end - start);
}

// Initialises a subterms list. It is assumed that the passed subterms list is
// uninitialised, and it is the caller's responsibility to free the list when
// it is no longer needed.
void subterms_list_init(subterms_list_t *list) {
	list->size = 0;
	list->capacity = 128;
	list->subterms = malloc(128 * sizeof(subterms_t));
}

// Frees a subterms list. The subterms list should not be used after this function is
// called, and it is the caller's responsibility to ensure that the list is
// initialised before it is used again.
void subterms_list_free(subterms_list_t *list) {
	list->size = 0;
	list->capacity = 0;
	free(list->subterms);
}

// Resizes a subterms list to the given capacity. The subterms list should be
// initialised before this function is called.
void subterms_list_resize(subterms_list_t *list, int capacity) {
	assert(list->capacity <= capacity);
	list->capacity = capacity;
	list->subterms = realloc(list->subterms, capacity * sizeof(subterms_t));
}

// Pushes a newly initialised subterms_t onto the end of the list.
void subterms_list_push(subterms_list_t *list) {
	if (list->size == list->capacity) {
		subterms_list_resize(list, list->capacity * 2);
	}
	list->size++;
	list->subterms[list->size - 1].size = 0;
}

// Returns the topmost subterm_t in the given list.
subterms_t *subterms_list_top(subterms_list_t *list) {
	assert(list->size > 0);
	return &list->subterms[list->size - 1];
}

// Lists all the redexes in a term. The subterms lists should be initialised
// before this function is called.
// TODO: we should make sure we never allow empty subterms
void list_redexes(term_t term, subterms_list_t *stack, subterms_list_t *results) {
	stack->size = 0;
	results->size = 0;
	if (term[0] == '\0') {
		return;
	}

	// Our strategy is to walk the term, pushing redexes onto the stack as we
	// encounter open brackets '('. When we encounter a close bracket ')', we
	// pop the top redex off the stack, and add the index to the previous redex.
	// If the popped redex is valid, we add it to the results list.
	int i = -1;
	subterms_list_push(stack);
	do { // hack so we handle '\0' explicitly
		i++;
		if (term[i] == '(') {
			subterms_push(subterms_list_top(stack), i);
			subterms_list_push(stack);
		} else if (term[i] == ')' || term[i] == '\0') {
			subterms_push(subterms_list_top(stack), i);
			subterms_t *subterm = subterms_list_top(stack);
			stack->size--;
			if (subterm->size > 0) {
				if ((subterm->size >= 3 && term[subterm->indices[0]] == 'I') ||
					(subterm->size >= 4 && term[subterm->indices[0]] == 'K') ||
					(subterm->size >= 5 && term[subterm->indices[0]] == 'S')) {
					subterms_list_push(results);
					memcpy(subterms_list_top(results), subterm, sizeof(subterms_t));
				}
			}
		} else {
			subterms_push(subterms_list_top(stack), i);
		}
	} while (term[i] != '\0');
}

// Applies a valid redex to a term, i.e. evaluates one of the combinators in
// the term. The term is modified in place, reallocating memory if necessary.
int apply_redex(term_t term, subterms_t *redex) {
	// We support three kinds of combinators, each with different arity:
	// 1. K: K x y = x
	// 2. S: S x y z = x z (y z)
	// 3. I: I x = x
	if (redex->size < 2) {
		return 0;
	}

	// If the first subterm is not a single character, and not equal to any of
	// the combinators, we can't apply the redex:
	if (redex->indices[0] + 1 != redex->indices[1]) {
		if (term[redex->indices[0]] != 'K' &&
			term[redex->indices[0]] != 'S' &&
			term[redex->indices[0]] != 'I') {
			return 0;
		}
	}
	
	// The size of the redex is the number of subterms it contains, plus one
	// for the end of the term. This tells us the arity of the redex:
	int reduced = 0;
	int arity = redex->size - 1;
	if (term[redex->indices[0]] == 'K' && arity >= 3) {
		char *subterm = term + redex->indices[0];
		char *x = term + redex->indices[1];
		int x_len = redex->indices[2] - redex->indices[1];
		memmove(subterm, x, x_len);
		char *rest = term + redex->indices[3];
		memmove(subterm + x_len, rest, strlen(rest)+1);
		reduced = 1;
	} else if (term[redex->indices[0]] == 'S' && arity >= 4) {
		char *x = term + redex->indices[1];		
		int x_len = redex->indices[2] - redex->indices[1];
		char *y = term + redex->indices[2];		
		int y_len = redex->indices[3] - redex->indices[2];
		char *z = term + redex->indices[3];		
		int z_len = redex->indices[4] - redex->indices[3];
		term = realloc(term, sizeof(char) * (strlen(term) + z_len + 2 + 1));
		char *rest = term + redex->indices[4];
		memmove(rest + z_len + 2, rest, strlen(rest)+1);
		memmove(x + x_len + y_len + z_len + 1, z, z_len);
		*(x + x_len + y_len + 2*z_len + 1) = ')';
		memmove(x + x_len + z_len + 1, y, y_len);
		*(x + x_len + z_len) = '(';
		memmove(x + x_len, x + x_len + y_len + z_len + 1, z_len);
		memmove(term + redex->indices[0], x, strlen(x)+1);
		reduced = 1;
	} else if (term[redex->indices[0]] == 'I' && arity >= 2) {
		char *subterm = term + redex->indices[0];
		char *x = term + redex->indices[1];
		int x_len = redex->indices[2] - redex->indices[1];
		memmove(subterm, x, x_len);
		char *rest = term + redex->indices[2];
		memmove(subterm + x_len, rest, strlen(rest)+1);
		reduced = 1;
	}

	if (reduced) {
		term_normalise(term);
	}
	return reduced;
}

int main() {
	term_t term = term_new("IS((SI)S)");
	printf("%s ->", term);
	term_normalise(term);
	printf("%s\n", term);
	return 0;

	//term_t term = term_new("SII(SI)SKS(KKI)");
	//subterms_list_t stack;
	//subterms_list_t results;
	//subterms_list_init(&stack);
	//subterms_list_init(&results);

	//// Calculate how many times we can call "list_redexes" in 1 second:
	//int i = 0;
	//clock_t start = clock();
	//do {
	//	list_redexes(term, &stack, &results);
	//	if (results.size == 0) {
	//		break;
	//	}
	//	apply_redex(term, &results.subterms[0]);
	//	printf("%s\n", term);
	//	i++;
	//} while (clock() - start < CLOCKS_PER_SEC);
	//printf("list_redexes: %d calls per second\n", i);
}
