#include "soup.h"
#include <stdio.h>

int main(int argc, char **argv) {
	term_t t = term_t_new("S(KI)(Ix)(Sxyz)");
	term_subterms_t *s = malloc(sizeof(term_subterms_t));
	term_subterms_t_init(s);

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

	term_t_free(t);
	term_subterms_t_free(s);
	return 0;
}
