# Turing Soup

The idea is to take a set of terms in the [combinator calculus](https://en.wikipedia.org/wiki/Combinatory_logic), and _react_ them to produce new sets of terms, while keeping the total number of base combinators in the soup constant. After reacting the terms together for a while, you start to see self-replicating terms popping up emergently, which is pretty cool. There are three possible reactions:

1) A term is _fused_ with another term, which means that we remove them both from the soup and add their concatenation back in.
2) A term is _fissioned_, which means that we remove it from the soup, split it randomly, and add the two components back in.
3) A term is _reduced_, which means a random redex (reducable subterm) in the term is chosen and evaluated.

Currently there are three different (partial) implementations of the Turing Soup. There's a python/C version in `src/soup.py` and `src/c_lib`, a C++ version in `src/cpp_soup` and a C version in `src/c_soup`.

Supported combinators (at least in the python version) are:
* `Ix -> x`
* `Kxy -> x`
* `Wxy -> xyy`
* `Sxyz -> xz(yz)`
* `Bxyz -> x(yz)`
* `Cxyz -> xzy`

This idea appears in a few places in the literature, but doesn't seem to be very well explored at all:
* [Combinatory Chemistry: Towards a Simple Model of Emergent Evolution](https://arxiv.org/abs/2003.07916)
* [Towards Complex Artificial Life](https://deepai.org/publication/towards-complex-artificial-life)
* [Programs as Polypeptides](https://direct.mit.edu/artl/article-abstract/22/4/451/2852/Programs-as-Polypeptides?redirectedFrom=fulltext)

## Python/C Version

Prototype, not very fast. Install dependencies with `poetry install` first, and then use one of the following commands:

1) `poetry run soup` to run a soup forever, printing out any self replicators that emerge in each generation:
  ```
  $ poetry run soup
STEP 0.
  immortals: []
STEP 1.
  immortals: []
STEP 2.
  immortals: []
...
  ```

  You can tweak parameters in `src/scripts/soup.py` if you like. Currently it's set up to use BCKW combinators instead of the usual SKI combinators since they're a bit easier to understand.
  
2) `poetry run comb beta <COMBINATOR>` to reduce a combinator term to beta normal form:
  ```
  $ poetry run comb beta 'S(K(SI))(S(KK)(SII))xy'
S(K(SI))(S(KK)(SII))xy
  -> K(SI)x(S(KK)(SII)x)y
  -> SI(KKx(SIIx))y
  -> Iy(K(Ix(Ix))y)
  -> y(xx)
  ```
  
3) `poetry run comb reduce <COMBINATOR>` to reduce a combinator by one step:
  ```
  $ poetry run comb reduce 'S(K(SI))(S(KK)(SII))xy'
S(K(SI))(S(KK)(SII))xy
  -> K(SI)x(S(KK)(SII)x)y
  ```
