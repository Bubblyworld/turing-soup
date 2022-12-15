#include "soup.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Initialises a subterms_t.
void subterms_init(subterms_t *subterms) {
	subterms->size = 0;
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
				subterms_list_push(results);
				memcpy(subterms_list_top(results), subterm, sizeof(subterms_t));
			}
		} else {
			subterms_push(subterms_list_top(stack), i);
		}
	} while (term[i] != '\0');
}

// TODO: this is just for testing:
int main() {
	term_t term = "W(WW)(WW)(WW)(WW)(WW)(WW)(WW)(WW)(WW)";
	subterms_list_t stack;
	subterms_list_t results;
	subterms_list_init(&stack);
	subterms_list_init(&results);

	// Calculate how many times we can call "list_redexes" in 1 second:
	int i = 0;
	clock_t start = clock();
	do {
		list_redexes(term, &stack, &results);
		i++;
	} while (clock() - start < CLOCKS_PER_SEC);
	printf("list_redexes: %d calls per second\n", i);
}
