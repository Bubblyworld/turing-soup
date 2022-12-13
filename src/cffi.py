from pathlib import Path
from cffi import FFI
from typing import Optional, Tuple
from copy import copy

def _clibpath(filename):
    return Path(__file__).parent / "c_lib" / filename

_COMB_H = _clibpath("comb.h")
_COMB_C = _clibpath("comb.c")
_COMB_BOOT = f"#include \"{_COMB_H}\""

_ffibuilder = FFI()
with open(_COMB_H) as f:
    _ffibuilder.cdef(f.read())
_ffibuilder.set_source("_comb", _COMB_BOOT, sources=[_COMB_C])
_ffibuilder.compile()

# Actual wrapper code is below. The goal in this module is to hide _all_ of
# the memory management from callers, as it's super easy to leak memory if you
# use the C API directly. The approach here is that whenever a pointer is
# returned from a function, it's immediately converted to a native Python
# wrapper that is responsible for freeing the memory. In the case of primitives
# like strings, we just convert them to their Python counterparts and free the
# memory immediately.
import _comb.lib as _lib

# Useful constants:
NULL = _ffibuilder.NULL

class Term:
    """Python wrapper of "term_t *" pointers that frees them with "free_term()"
    when they are garbage-collected."""
    def __init__(self, term: "term_t *"):
        self._term = term

    def __del__(self):
        _lib.free_term(self._term)

    def __repr__(self):
        return f"Term({self})"

    def __str__(self):
        c_str = _lib.print_term(self._term)
        py_str = _ffibuilder.string(c_str).decode("utf-8")
        _lib.free(c_str)
        return py_str

    def __eq__(self, other):
        if not isinstance(other, Term):
            return False
        return str(self) == str(other)

    def __hash__(self):
        return hash(str(self))

    def __bool__(self):
        return self._term != NULL

    def __nonzero__(self):
        return self.__bool__()

    def __deepcopy__(self, memo):
        return Term(lib.copy_term(self._term))

    def __copy__(self):
        return Term(lib.copy_term(self._term))

    def reduce(self) -> "Term":
        """Returns the result of reducing this term, or None if it is already
        in normal form."""
        return reduce(self)

    def beta_normal(self, cutoff=10000) -> Tuple["Term", bool]:
        """Returns the beta normal form of this term, and whether or not the
        cutoff number of reductions was reached. If the cutoff was reached, the
        return value is None."""
        term = self
        for _ in range(cutoff):
            reduced = term.reduce()
            if reduced == term:
                return term, True
            term = reduced
        return None, False

    def is_beta_normal(self) -> bool:
        """Returns True if this term is in beta normal form."""
        return self == self.reduce()

def leaf(c: str) -> Term:
    """Creates a new leaf node with the given character.

        Args:
            c: The character to store in the leaf.

        Returns:
            A Term representing the new leaf.
        """
    return Term(_lib.new_leaf(bytes(c, "utf-8")))

def internal(left: Term, right: Term) -> Term:
    """Creates a new internal node with the given children.

        Args:
            left: The left child of the new node.
            right: The right child of the new node.

        Returns:
            A Term representing the new node.
        """
    return Term(_lib.new_node(copy(left)._term, copy(right)._term))

def parse(str: str) -> Term:
    """Parses the given string into a term. The string must be a valid term,
    with balanced parentheses and no whitespace, otherwise an error will be
    raised.

        Args:
            str: The string to parse.

        Returns:
            A Term representing the parsed string.

        Raises:
            ValueError: If the string is not a valid term.
        """
    term = _lib.parse_term(bytes(str, "utf-8"))
    if term == NULL:
        raise ValueError(f"Invalid term: {str}")
    return Term(term)

def reduce(term: Term) -> Term:
    """Reduces every redex in the given term by a single step, starting from
    the most deeply-nested redex.

        Args:
            term: The term to reduce.

        Returns:
            A Term representing the result of reducing the given term.
        """
    return Term(_lib.reduce_term(term._term))
