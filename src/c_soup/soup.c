#include "soup.h"

// Normalises the given term by removing all unnecessary parentheses. Note that
// two terms can have different normal forms, and yet still behave in exactly
// the same way: S(KI)Ix = KIx(Ix) = I(Ix) = Ix, for instance.
void normalise(term_t term) {
}

// Writes all of the redexes in the given term to s->redexes, and returns
// true if any redexes were found.
bool redexes(term_t term, term_subterms_t *s) {
	term_subterms_t_reset(s);	
	if (term == NULL || term[0] == '\0') {
		return false;
	}

	int i = -1;
	subterms_t_push(s->subterms);
	do { // hack so we explicitly handle the '\0' char
		i++;
		if (term[i] == '(') {
			indices_t_push(subterms_t_top(s->subterms));
			*indices_t_top(subterms_t_top(s->subterms)) = i;
			subterms_t_push(s->subterms);
		} else if (term[i] == ')' || term[i] == '\0') {
			indices_t_push(subterms_t_top(s->subterms));
			*indices_t_top(subterms_t_top(s->subterms)) = i;
			indices_t *pop = subterms_t_pop(s->subterms);
			if (subterm_is_redex(term, pop)) {
				subterms_t_push(s->redexes);
				indices_t_copy(pop, subterms_t_top(s->redexes));
			}
		} else {
			indices_t_push(subterms_t_top(s->subterms));
			*indices_t_top(subterms_t_top(s->subterms)) = i;
		}
	} while (term[i] != '\0');

	return s->redexes->size > 0;
}
