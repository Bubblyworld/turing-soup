from .cffi import parse, reduce, Term
from typing import List, Mapping
import random

class Soup:
    """A Soup is a collection of combinator terms that can interact with each
    other. There's a paper in the literature on a similar concept, called the
    "Combinator Chemistry", but the basic idea is that in each step of the
    Soup simulation, a statistical subset of terms each have one of three
    actions applied to them:
      1) Reduction: if the term can be reduced, it is reduced by one step.
      2) Fission: the term is randomly split into two terms.
      3) Fusion: another term is randomly selected and fused with this term.

    The hope is that after some time, interesting behaviours such as self-
    replication will emerge automatically from this dynamics.
    """

    # Constants that can be tuned to find interesting behaviours:
    P_ACTION = 0.4 # Probability of applying an action to a term.
    P_REDUCE = 0.75 # Probability of the action being a reduction.
    P_FISSION = 0.125 # Probability of the action being a fission.
    P_FUSION = 0.125 # Probability of the action being a fusion.

    def __init__(self, terms: int, alphabet: str = "SKI"):
        """Creates a new Soup with the given number and type of atomic terms,
        i.e. combinators.

        Args:
            terms: The number of atomic terms (i.e. combinators) to create.
            alphabet: The alphabet of atomic terms (i.e. combinators) to use.
        """
        self._terms = terms
        self._alphabet = alphabet
        self._soup = [parse(random.choice(alphabet)) for _ in range(terms)]

    def __str__(self):
        """Returns a string representation of the Soup.
        """
        return str([str(term) for term in self._soup])

    def step(self):
        """Performs one step of the Soup simulation, applying actions to
        a random subset of terms in the Soup.
        """
        soup = []
        random.shuffle(self._soup)
        pre_count = self._count()
        while len(self._soup) > 0:
            term = self._soup.pop()
            if random.random() < self.P_ACTION:
                p = random.random()
                if p < self.P_REDUCE:
                    soup.append(reduce(term))
                elif p < self.P_REDUCE + self.P_FISSION:
                    soup.extend(self._fission(term))
                else:
                    if len(self._soup) == 0:
                        soup.append(term)
                    else:
                        soup.append(parse(str(self._soup.pop()) + str(term)))
            else:
                soup.append(term)
        self._soup = soup

        # Insert any deficit back into the soup as atomic terms:
        # There may occasionally be a sufficit from S terms.
        post_count = self._count()
        for c in self._alphabet:
            if c not in pre_count:
                pre_count[c] = 0
            if c not in post_count:
                post_count[c] = 0
            deficit = pre_count[c] - post_count[c]
            for _ in range(deficit):
                self._soup.append(parse(c))

    def _count(self) -> Mapping[str, int]:
        """Returns a count of each kind of combinator in the soup.

        Returns:
            A dictionary mapping each combinator to the number of times it
            appears in the term.
        """
        count: Mapping[str, int] = {}
        for term in self._soup:
            for c in str(term):
                if c == "(" or c == ")":
                    continue
                if c in count:
                    count[c] += 1
                else:
                    count[c] = 1
        return count

    # TODO: can we split inside the term too? Could be cool.
    def _fission(self, term: Term) -> List[Term]:
        """Splits the given term into two terms, if possible.

        Args:
            term: The term to split.

        Returns:
            The left child of the split, or the original term if it could not
            be split.
        """
        # We can split a term anywhere that doesn't unbalance it's parentheses:
        term_str = str(term)
        splits = []
        depth = 0
        for i, c in enumerate(term_str):
            if c == "(":
                depth += 1
            elif c == ")":
                depth -= 1
            elif depth == 0 and i > 0 and i < len(term_str) - 1:
                splits.append(i)

        # If there are no valid splits, we can't split the term:
        if len(splits) == 0:
            return [term]

        # Otherwise, we pick a random split and split the term:
        split = random.choice(splits)
        return [parse(term_str[:split]), parse(term_str[split:])]