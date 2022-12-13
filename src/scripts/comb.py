from src.cffi import parse, reduce
import argparse

def main():
    parser = argparse.ArgumentParser()
    parser.set_defaults(func=lambda _: parser.print_help())
    subparsers = parser.add_subparsers()
    reduce_parser = subparsers.add_parser("reduce")
    reduce_parser.add_argument("term")
    reduce_parser.set_defaults(func=reduce_term)
    beta_parser = subparsers.add_parser("beta")
    beta_parser.add_argument("term")
    beta_parser.set_defaults(func=beta_term)
    args = parser.parse_args()
    args.func(args)

def reduce_term(args):
    term = parse(args.term)
    print(term)
    term = reduce(term)
    print(f"  -> {term}")

def beta_term(args):
    term = parse(args.term)
    print(term)
    while not term.is_beta_normal():
        term = reduce(term)
        print(f"  -> {term}")
