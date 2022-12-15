#pragma once

// Combinator terms are simply C strings:
typedef char *term_t;

// Subterms keeps track of the indices of successive subterms in a term. These
// are used to check for redexes and for fast evaluation.
typedef struct {
	int size;       // The number of set subterms.
	int indices[6]; // We never need more than 6 subterms for redexes.
} subterms_t;

// Functions for working with subterms:
void subterms_init(subterms_t *subterms);
void subterms_push(subterms_t *subterms, int index);

/******************************************************************************
 *                       LISTING REDEXES IN A TERM														*
 * Our general philosophy in this implementation is to avoid allocations as	  *
 * much as possible. Thus for listing redexes in a term, the user passes in	  *
 * input/output structures that are reused for each call, and reallocated as  *
 * necessary if they are too small. This is a bit more complicated than		    *
 * simply returning a vector of redexes, but it's much more efficient.			  *
 ******************************************************************************/

typedef struct {
	int size;            // The number of subterms_t currently in the list.
	int capacity;        // The number of subterms_t that can be stored.
	subterms_t *subterms; // The list of subterms_t.
} subterms_list_t;

// Functions for working with subterms_list__t:
void subterms_list_init(subterms_list_t *list);
void subterms_list_free(subterms_list_t *list);
void subterms_list_resize(subterms_list_t *list, int capacity);
void subterms_list_push(subterms_list_t *list);
subterms_t *subterms_list_top(subterms_list_t *list);

// List all the redexes in a term:
void list_redexes(term_t term, subterms_list_t *stack, subterms_list_t *results);
