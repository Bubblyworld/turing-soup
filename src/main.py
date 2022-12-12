import src.comb as comb
import time

terms = [
    "Sx(Ky)z",
    "B(BW)(BBC)xyz",
    "S(KS)Kxyz",
    "S(BBS)(KK)xyz",
    "S(S(KS)K(S(KS)K)S)(KK)xyz",
    "S(S(KS)K(S(KS)K)S)(KK)",
]

def main():
    for term in terms:
        if term != terms[0]:
            print()

        last_tree = comb.parse_term(term)
        print(comb.print_term(last_tree))
        while True:
            tree = comb.reduce_term(last_tree)
            if comb.print_term(tree) == comb.print_term(last_tree):
                break
            comb.free_term(last_tree)
            last_tree = tree
            print("-> ", comb.print_term(tree))
        comb.free_term(last_tree)
