from pathlib import Path
from cffi import FFI
from typing import Optional

def _localpath(filename):
    return Path(__file__).parent / filename

_COMB_H = _localpath("comb.h")
_COMB_C = _localpath("comb.c")
_COMB_BOOT = f"#include \"{_COMB_H}\""

_ffibuilder = FFI()
with open(_COMB_H) as f:
    _ffibuilder.cdef(f.read())
_ffibuilder.set_source("_comb", _COMB_BOOT, sources=[_COMB_C])
_ffibuilder.compile()

# Copilot, this is the header file we are wrapping:
"""
/* Terms are represented as strings over the alphabet 'SKIBCW()abc...z', and
 * are required to have balanced parentheses. Uppercase characters are
 * the combinators, and lowercase characters are variables. Terms are 
 * evaluated as left-associative, with brackets to override this, meaning 
 * that a term like 'Sa(bc)' is equivalent to '((Sa)(bc))'.
 *
 * The supported combinators, along with their derivation schemas, are:
 *   Sxyz -> xz(yz)
 *   Kxy  -> x
 *   Ix   -> x
 *   Bxyz -> x(yz)
 *   Cxyz -> xzy
 *   Wxy  -> xyy
 *
 * The internal representation of terms is a tree of nodes, where each node
 * is either a combinator or a variable. The tree is constructed by parsing
 * the string representation of the term, and then evaluated by traversing
 * the tree left-to-right and applying the appropriate combinators.
 */

typedef struct term {
		char c; // '\0' unless is_leaf
	  int is_leaf; // 0 if internal node, 1 if leaf
		struct term *left; // NULL if leaf, not NULL otherwise
		struct term *right; // NULL if leaf, optionally NULL otherwise
} term_t;

term_t *new_leaf(char c);
term_t *new_node(term_t *left, term_t *right);
void free_term(term_t *term);
char *print_term(term_t *term);
term_t *parse_term(const char *str);
term_t *reduce_term(term_t *term);
"""

# I want you to wrap each of these methods, with the following rules:
#   1) every function must have a docstring
#   2) every function must have a type annotation
#   3) functions that take or return strings must have a python interface,
#      with the funky _ffibuilder.string() stuff hidden from the user.
import _comb.lib as _lib

NULL = _ffibuilder.NULL

def new_leaf(c: str) -> "term_t *":
    """Creates a new leaf node with the given character.

        Args:
            c: The character to store in the leaf.

        Returns:
            A cffi pointer to the new leaf node.
        """
    return _lib.new_leaf(bytes(c, "utf-8"))

def new_node(left: "term_t *", right: "term_t *") -> "term_t *":
    """Creates a new internal node with the given children.

        Args:
            left: The left child of the new node.
            right: The right child of the new node.

        Returns:
            A cffi pointer to the new internal node.
        """
    return _lib.new_node(left, right)

def free_term(term: "term_t *") -> None:
    """Frees the memory associated with the given term.

        Args:
            term: The term to free.
        """
    _lib.free_term(term)

def print_term(term: "term_t *") -> str:
    """Prints the given term to a string.

        Args:
            term: The term to print.

        Returns:
            A string representation of the term.
        """
    return _ffibuilder.string(_lib.print_term(term)).decode("utf-8")
