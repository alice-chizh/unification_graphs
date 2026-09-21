//#include <iostream>
#include <sstream>
#include "Term.h"

std::shared_ptr<Term> Term::Var(const std::string& name){
    auto t = std::make_shared<Term>();
    t->type = TermType::VARIABLE;
    t->name = name;
    return t;
};
std::shared_ptr<Term> Term::Fun(const std::string& name, const std::vector<std::shared_ptr<Term>>& args) {
    auto t = std::make_shared<Term>();
    t->type = TermType::FUNCTION;
    t->name = name;
    t->args = args;
    return t;
};

bool Term::isVariable() const {
    return type == TermType::VARIABLE; //if (term->type == TermType::VARIABLE)
}

bool Term::isFunction() const {
    return type == TermType::FUNCTION;
}

std::string Term::toString() const {
    if (isVariable()) {
        return name; }
    if (args.empty()) {
        return name; }
    std::stringstream result;
    result << name << "(";
    for (size_t i = 0; i < args.size(); ++i) {
        if (i > 0) {
            result << ", ";
        }
        result << args[i]->toString();
    }
    result << ")";
    return result.str();
}





/*
struct Equation<Term>  {
    shared_ptr<Term> left;
    shared_ptr<Term> right;
};

deque<Equation> equations;
equations.push_back({left, right});



unordered_map<string, shared_ptr<Term>> substitution;

bool occurs (const string& variable, const shared_ptr<Term>& term) {

}


bool occurs(
        const string& variable,
        const shared_ptr<Term>& term
) {
    if (term->type == Term::VARIABLE) {
        return term->name == variable;
    }

    for (const auto& arg : term->args) {
        if (occurs(variable, arg)) {
            return true;
        }
    }

    return false;
}










bool unify(
        deque<Equation>& equations,
        unordered_map<string, shared_ptr<Term>>& substitution
) {
    while (!equations.empty()) {

        Equation eq = equations.front();
        equations.pop_front();

        auto s = eq.left;
        auto t = eq.right;

        // DELETE
        if (s == t) {
            continue;
        }

        // Если переменная слева
        if (s->type == Term::VARIABLE) {

            const string& x = s->name;

            // X = X
            if (t->type == Term::VARIABLE &&
                t->name == x) {
                continue;
            }

            // OCCURS CHECK
            if (occurs(x, t)) {
                return false;
            }

            // ELIMINATE
            substitution[x] = t;

            // Заменяем X на t во всех оставшихся уравнениях
            for (auto& e : equations) {
                // здесь нужна applySubstitution
            }

            continue;
        }

        // Если переменная справа:
        // s = X превращаем в X = s
        if (t->type == Term::VARIABLE) {
            equations.push_back({t, s});
            continue;
        }

        // CLASH
        if (s->name != t->name ||
            s->args.size() != t->args.size()) {
            return false;
        }

        // DECOMPOSE
        for (size_t i = 0; i < s->args.size(); ++i) {
            equations.push_back({
                                        s->args[i],
                                        t->args[i]
                                });
        }
    }

    return true;
}
*/