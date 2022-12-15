#pragma once

#include <string>
#include <vector>
using namespace std;

typedef string Term;

struct Redex {
	int index;
	vector<Term> inputs;
	vector<Term> outputs;
};

bool validateTerm(const Term &term);
vector<Redex> listRedexes(const Term &term);
Term applyRedex(const Term &term, const Redex &redex);
