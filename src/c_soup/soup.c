#include "soup.h"
#include <stdlib.h>

// Normalises the given term by removing all unnecessary parentheses. Note that
// two terms can have different normal forms, and yet still behave in exactly
// the same way: S(KI)Ix = KIx(Ix) = I(Ix) = Ix, for instance. This operates
// in-place.
void normalise(term_t term, state_t *s) {
  state_t_reset(s);
  if (term == NULL || term[0] == '\0') {
    return;
  }

  int i = -1;
  subterms_t_push(s->stack);
  indices_t_push(subterms_t_top(s->stack));
  *indices_t_top(subterms_t_top(s->stack)) = i;
  do { // hack so we explicitly handle the '\0' char
    i++;
    if (term[i] == '(') {
      subterms_t_push(s->stack);
      indices_t_push(subterms_t_top(s->stack));
      *indices_t_top(subterms_t_top(s->stack)) = i;
    } else if (term[i] == ')' || term[i] == '\0') {
      indices_t_push(subterms_t_top(s->stack));
      *indices_t_top(subterms_t_top(s->stack)) = i;
      indices_t *pop = subterms_t_pop(s->stack);

      if (pop->data[1] - pop->data[0] <= 2 ||
          pop->data[0] == subterms_t_top(s->stack)->data[0] + 1) {
        char *start = &term[pop->data[0]];
        char *end = term + pop->data[1];
        memcpy(end, end+1, strlen(end+1)+1);
        memcpy(start, start+1, strlen(start+1)+1);
        i -= 2;
      }
    }
  } while (term[i] != '\0');
}

// Writes all of the redexes in the given term to s->redexes, and returns
// true if any redexes were found.
bool redexes(term_t term, state_t *s) {
  state_t_reset(s);  
  if (term == NULL || term[0] == '\0') {
    return false;
  }

  int i = -1;
  subterms_t_push(s->stack);
  do { // hack so we explicitly handle the '\0' char
    i++;
    if (term[i] == '(') {
      indices_t_push(subterms_t_top(s->stack));
      *indices_t_top(subterms_t_top(s->stack)) = i;
      subterms_t_push(s->stack);
    } else if (term[i] == ')' || term[i] == '\0') {
      indices_t_push(subterms_t_top(s->stack));
      *indices_t_top(subterms_t_top(s->stack)) = i;
      indices_t *pop = subterms_t_pop(s->stack);
      if (subterm_is_redex(term, pop)) {
        subterms_t_push(s->redexes);
        indices_t_copy(pop, subterms_t_top(s->redexes));
      }
    } else {
      indices_t_push(subterms_t_top(s->stack));
      *indices_t_top(subterms_t_top(s->stack)) = i;
    }
  } while (term[i] != '\0');

  return s->redexes->size > 0;
}

// Applies the given redex to the term in-place.
void apply_redex(term_t term, indices_t *indices, state_t *s) {
  char *subterm = term + indices->data[0];
  if (subterm[0] == 'I') {
    // Ix = x
    char *x = term + indices->data[1];
    memcpy(subterm, x, strlen(x) + 1);
  } else if (subterm[0] == 'K') {
    // Kxy = x
    char *x = term + indices->data[1];
    char *y = term + indices->data[2];
    char *rest = term + indices->data[3];
    memcpy(y, rest, strlen(rest) + 1);
    memcpy(subterm, x, strlen(x) + 1);
  } else if (subterm[0] == 'S') {
    // Sxyz = (xz)(yz)
    char *x = term + indices->data[1];
    char *y = term + indices->data[2];
    char *z = term + indices->data[3];
    char *rest = term + indices->data[4];
    int x_len = y - x;
    int y_len = z - y;
    int z_len = rest - z;
    term = realloc(term, sizeof(char) * (strlen(term) + z_len + 3));
    memmove(rest + z_len + 2, rest, strlen(rest) + 1);
    memmove(x + x_len + y_len + z_len + 1, z, z_len);
    *(x + x_len + y_len + 2*z_len + 1) = ')';
    memmove(x + x_len + z_len + 1, y, y_len);
    *(x + x_len + z_len) = '(';
    memmove(x + x_len, x + x_len + y_len + z_len + 1, z_len);
    memmove(subterm, x, strlen(x) + 1);
  }

  normalise(term, s);
}
