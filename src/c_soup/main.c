#include "soup.h"
#include <stdio.h>

int main(int argc, char **argv) {
  term_t t = term_t_new("I(SII)(SII)");
  state_t *s = malloc(sizeof(state_t));
  state_t_init(s);

  // Compute the redexes in t and print them:
  redexes(t, s);
  for (int i = 0; i < s->redexes->size; i++) {
    indices_t *redex = &s->redexes->data[i];
    for (int j = 0; j < redex->size; j++) {
      printf("%d ", redex->data[j]);
    }
    term_t st = subterm(t, &s->redexes->data[i]);
    printf("-> %s", st);
    term_t_free(st);
    printf("\n");
  }

  // Apply the first redex and print the resulting term:
  apply_redex(t, &s->redexes->data[0], s);
  printf("%s\n", t);

  term_t_free(t);
  state_t_free(s);
  return 0;
}
