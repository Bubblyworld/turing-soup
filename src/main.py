import src.comb as comb
import time

term_strs = [
    "Sx(Ky)z",
    "B(BW)(BBC)xyz",
    "S(KS)Kxyz",
    "S(BBS)(KK)xyz",
    "S(S(KS)K(S(KS)K)S)(KK)xyz",
    "S(S(KS)K(S(KS)K)S)(KK)",
    "SII(SII)",
]

def main():
    for term_str in term_strs:
        if term_str != term_strs[0]:
            print()

        term = comb.parse(term_str)
        print(term)
        while True:
            termR = comb.reduce(term)
            if term == termR:
                break
            else:
                term = termR
                print(f"-> {term}")
