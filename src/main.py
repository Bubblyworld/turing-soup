import src.comb as comb

def main():
    n = comb.new_node(comb.new_leaf("a"), comb.NULL)
    print(comb.print_term(n))
    comb.free_term(n)

    # Now try a more interesting example, 'Sx(Ky)z':
    n = comb.new_node(
        comb.new_node(
            comb.new_node(
                comb.new_leaf("S"),
                comb.new_leaf("x"),
            ),
            comb.new_node(
                comb.new_leaf("K"),
                comb.new_leaf("y"),
            ),
        ),
        comb.new_leaf("z"),
    )
    print(comb.print_term(n))
    comb.free_term(n)
