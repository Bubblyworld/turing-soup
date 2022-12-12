import src.comb as comb
import time

def main():
    term = comb.parse_term("((a)(b))")
    print(comb.print_term(term))
    comb.free_term(term)

    term = comb.parse_term("Sx(Ky)z")
    print(comb.print_term(term))
    comb.free_term(term)

    comb.parse_term("(a()") # should raise
