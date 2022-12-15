#include "soup.h"
#include <iostream>
#include <vector>
#include <stack>
#include <set>
#include <map>
#include <queue>
using namespace std;

// The set of supported combinators.
// https://en.wikipedia.org/wiki/Combinatory_logic
const set<char> combinators = {'S', 'K', 'I', 'B', 'C', 'W'};

// A term is valid if the only characters are combinators and parentheses,
// and the parentheses are balanced and nested correctly.
bool validateTerm(const Term &term) {
		int balance = 0;
		for (char c : term) {
				if (c == '(') {
						balance++;
				} else if (c == ')') {
						balance--;
				} else if (combinators.find(c) == combinators.end()) {
						return false;
				}
				if (balance < 0) {
						return false;
				}
		}
		return balance == 0;
}

struct _ListRedexCtx {
		int start;
		vector<string> terms;
};

// Terms are left-associative (i.e. "abc" and "((ab)c)" are equivalent), so we
// can find redexes by going left-to-right. A redex is a combinator followed
// by a certain number of arguments, such as "Sxyz == (((Sx)y)z)". Note that
// by left-associativity, something like aSbc is _not_ a redex, because of the
// implicit parentheses: "aSbc" == "(aS)bc".
vector<Redex> listRedexes(const Term &term) {
	if (term.length() == 0) {
		return vector<Redex>();
	}

	vector<Redex> redexes;
	auto handleRedex = [&](const _ListRedexCtx &ctx) {
		// There must be at least one term and the first must be a combinator:
		if (ctx.terms.size() == 0) {
			return;
		}
		if (ctx.terms[0].length() != 1) {
			return;
		}

		// Combinators are defined by their reduction rules:
		//   Sxyz  -> xz(yz)
		//   Kxy   -> x
		//   Ix    -> x
		//   Bxyz  -> x(yz)
		//   Cxyz  -> xzy
		//   Wxy   -> xyy
		// ...thus the prospective redex is valid if the number of arguments
		// is at least the combinator's arity.
		vector<Term> inputs;
		vector<Term> outputs;
		if (ctx.terms[0] == "S") {
			if (ctx.terms.size() < 4) {
				return;
			}
			inputs.push_back(Term(ctx.terms[3])); // S consumes an additional 'z'
			outputs.push_back(Term("S")); // S ejects 'S'
		} else if (ctx.terms[0] == "K") {
			if (ctx.terms.size() < 3) {
				return;
			}
			outputs.push_back(Term("K")); // K ejects 'K' and an additional 'y'
			outputs.push_back(Term(ctx.terms[2]));
		} else if (ctx.terms[0] == "I") {
			if (ctx.terms.size() < 2) {
				return;
			}
			outputs.push_back(Term("I")); // I ejects 'I'
		} else if (ctx.terms[0] == "B") {
			if (ctx.terms.size() < 3) {
				return;
			}
			outputs.push_back(Term("B")); // B ejects 'B'
		} else if (ctx.terms[0] == "C") {
			if (ctx.terms.size() < 3) {
				return;
			}
			outputs.push_back(Term("C")); // C ejects 'C'
		} else if (ctx.terms[0] == "W") {
			if (ctx.terms.size() < 3) {
				return;
			}
			inputs.push_back(Term(ctx.terms[2])); // W consumes an additional 'y'
			outputs.push_back(Term("W")); // W ejects 'W'
		} else {
			return; // not a combinator, should never happen
		}

		// The redex is valid, so add it to the list:
		Redex redex;
		redex.index = ctx.start;
		redex.inputs = inputs;
		redex.outputs = outputs;
		redexes.push_back(redex);
	};

	stack<_ListRedexCtx> stack;
	stack.push({_ListRedexCtx{0, vector<string>()}});
	for (int i = 0; i < term.length(); i++) {
		char c = term[i];
		if (c == '(') {
			stack.push({_ListRedexCtx{i + 1, vector<string>()}});
		} else if (c == ')') {
			_ListRedexCtx ctx = stack.top();
			stack.pop();
			handleRedex(ctx);

			string subterm = term.substr(ctx.start, i - ctx.start);
			stack.top().terms.push_back(subterm);
		} else {
			stack.top().terms.push_back(string(1, c));
		}
	}

	handleRedex(stack.top());
	return redexes;
}

Term applyRedex(const Term &term, const Redex &redex) {
	return Term();
}

int main() {
	Term term = "SII(KSI)(IK)";
	if (!validateTerm(term)) {
		cout << "Invalid term: " << term << endl;
		return 1;
	}
	cout << "Valid term: " << term << endl;
	vector<Redex> redexes = listRedexes(term);
	for (Redex redex : redexes) {
		cout << "Redex at " << redex.index << endl;
		cout << "  Inputs:" << endl;
		for (Term input : redex.inputs) {
			cout << "    " << input << endl;
		}
		cout << "  Outputs:" << endl;
		for (Term output : redex.outputs) {
			cout << "    " << output << endl;
		}
	}

	// Now time how long it takes to run listRedexes 1million times:
	auto start = chrono::high_resolution_clock::now();
	for (int i = 0; i < 1000000; i++) {
		listRedexes(term);
	}
	auto end = chrono::high_resolution_clock::now();
	auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
	cout << "Time taken: " << duration.count() << " microseconds" << endl;
}
