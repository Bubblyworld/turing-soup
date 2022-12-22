#include "soup.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char **argv) {
  state_t *s = malloc(sizeof(state_t));
  state_t_init(s);

  // First experiment is to see if it successfully reduces the W combinator:
  {
    term_t t = term_t_new(argv[1]);
    normalise(t, s);
    printf("%s\n", t);
    t = reduce(t, s);
    printf("-> %s\n", t);
    term_t_free(t);
  }

  // Time how many calls to apply can be made in 1 second:
  {
    int count = 0;
    clock_t start = clock();
    for (int i = 0; i < 1000000; i++) {
      term_t t = term_t_new(argv[1]);
      normalise(t, s);
      while (redexes(t, s)) {
        t = apply(t, subterms_t_top(s->redexes), s);
        count++;
      }
      term_t_free(t);
    }

    clock_t end = clock();
    double time = (double)(end - start) / CLOCKS_PER_SEC;
    double rate = count / time;
    printf("Applied %d redexes in %f seconds (%f redexes per second)\n", count, time, rate);
  }

  state_t_free(s);
  return 0;
}
