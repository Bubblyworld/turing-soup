#include "soup.h"
#include <stdio.h>

int main(int argc, char **argv) {
  term_t t = term_t_new("SII(SII)");
  state_t *s = malloc(sizeof(state_t));
  state_t_init(s);
  normalise(t, s);

  // Keep applying the first redex until there are no more (beta reduction):
  printf("%s\n", t);
  while (redexes(t, s)) {
    t = apply_redex(t, subterms_t_top(s->redexes), s);
    printf("-> %s\n", t);
  }

  term_t_free(t);
  state_t_free(s);
  return 0;
}
