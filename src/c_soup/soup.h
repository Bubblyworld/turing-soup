#pragma once
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/******************************************************************************
 *                   UTILITY FUNCTIONS AND DATA STRUCTURES                    *
 ******************************************************************************/

// I get annoyed by C's lack of boolean types, so I'm hiding it:
typedef int bool;
#define true 1
#define false 0

// A simple stack data structure that resizes itself as necessary. Useful as
// cached working memory for term manipulation to avoid allocations.
#define STACK(type, name) \
  typedef struct { \
    type *data; \
    int size; \
    int capacity; \
  } name; \
  inline static void name##_init(name *s) { \
    s->data = malloc(sizeof(type) * 16); \
    s->size = 0; \
    s->capacity = 16; \
    for (int i = 0; i < s->capacity; i++) { \
      type##_init(&s->data[i]); \
    } \
  } \
  inline static void name##_reset(name *s) { \
    s->size = 0; \
  } \
  inline static void name##_free(name *s) { \
    for (int i = 0; i < s->capacity; i++) { \
      type##_free(&s->data[i]); \
    } \
    free(s->data); \
  } \
  inline static void name##_push(name *s) { \
    if (s->size == s->capacity) { \
      int new_capacity = s->capacity ? s->capacity * 2 : 1; \
      s->data = (type*) realloc(s->data, new_capacity * sizeof(type)); \
      for (int i = s->capacity; i < new_capacity; i++) { \
        type##_init(&s->data[i]); \
      } \
      s->capacity = new_capacity; \
    } \
    s->size++; \
    type##_reset(&s->data[s->size - 1]); \
  } \
  inline static void name##_copy(name *src, name *dst) { \
    name##_reset(dst); \
    for (int i = 0; i < src->size; i++) { \
      name##_push(dst); \
      type##_copy(&src->data[i], &dst->data[i]); \
    } \
  } \
  inline static type *name##_pop(name *s) { \
    return &s->data[--s->size]; \
  } \
  inline static type *name##_top(name *s) { \
    return &s->data[s->size - 1]; \
  }

/******************************************************************************
 *                            LOW-LEVEL SOUP API                              *
 ******************************************************************************/

// Terms are strings of combinators, variables and parentheses. Terms are bound
// by left-associativity, so the term "ab(c)" is equivalent to "(ab)c", with
// parentheses being used to override this. We represent terms internally as
// C strings for efficiency.
typedef char *term_t;
inline static term_t term_t_new(const char *str) {
  return strdup(str);
}
inline static void term_t_free(term_t term) {
  free(term);
}
void normalise(term_t term); // IMPLEMENT ME

// When working with terms, we often need to store the set of indices of
// successive subterms within the term, so that we can easily compute
// combinator reductions. We represent these indices as a stack of integers.
// TODO: these int_XXX methods can probably be handled by the macro, and it
//       might even be faster to do that anyway.
inline static void int_init(int *i) { *i = 0; } // for STACK
inline static void int_reset(int *i) { *i = 0; } // for STACK
inline static void int_free(int *i) {} // for STACK
inline static void int_copy(int *src, int *dst) { *dst = *src; } // for STACK
STACK(int, indices_t);
inline term_t subterm(term_t term, indices_t *indices) {
  int start = indices->data[0];
  int end = indices->data[indices->size - 1];
  char *subterm = malloc(end - start + 1);
  memcpy(subterm, term + start, end - start);
  subterm[end - start] = '\0';
  return subterm;
}
inline static int subterm_length(term_t term, indices_t *indices, int index) {
  int start = indices->data[index];
  int end = index == indices->size - 1 ? strlen(term) : indices->data[index + 1];
  return end - start;
}
inline static bool subterm_is_redex(term_t term, indices_t *indices) {
  return indices->size > 1 && subterm_length(term, indices, 0) == 1 && (
    (term[indices->data[0]] == 'S' && indices->size >= 5) || 
    (term[indices->data[0]] == 'K' && indices->size >= 4) ||
    (term[indices->data[0]] == 'I' && indices->size >= 3)
  );
}

// The first useful thing we can do with terms is calculate their sets of
// subterms, as well as their sets of redexes. To make this really fast we
// keep a cached pair of stacks as working memory for the algorithms:
STACK(indices_t, subterms_t);
typedef struct {
  subterms_t *subterms;
  subterms_t *redexes;
} term_subterms_t;
inline static void term_subterms_t_init(term_subterms_t *t) {
  t->subterms = malloc(sizeof(subterms_t));
  t->redexes = malloc(sizeof(subterms_t));
  subterms_t_init(t->subterms);
  subterms_t_init(t->redexes);
}
inline static void term_subterms_t_free(term_subterms_t *t) {
  subterms_t_free(t->subterms);
  subterms_t_free(t->redexes);
}
inline static void term_subterms_t_reset(term_subterms_t *t) {
  subterms_t_reset(t->subterms);
  subterms_t_reset(t->redexes);
}
// Constraint: must return true if there were any redexes, false otherwise.
bool redexes(term_t term, term_subterms_t *s); // IMPLEMENT ME

/******************************************************************************
 *                       EVALUATING A REDEX IN A TERM                          *
 ******************************************************************************/

//int apply_redex(term_t term, subterms_t *redex);
