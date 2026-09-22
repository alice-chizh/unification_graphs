#include "Unifier.h"
#include <iostream>

bool Unifier::unify(std::shared_ptr<Term> left, std::shared_ptr<Term> right) {
    equations.clear();
    substitution.clear();
    equations.push_back({left, right});

    while (!equations.empty()) { //пока очередь уравнений не пуста
        Equation equation = equations.front();
        equations.pop_front();
        auto l = substitute(equation.left);
        auto r = substitute(equation.right);

        // DELETE t = t
        if (equalTerms(l, r)) {
            continue;
        }

        // ORIENT t = X  ->  X = t
        if (r->isVariable() && !l->isVariable()) {
            std::swap(l, r); }

        // Если обе стороны переменные
        if (l->isVariable() && r->isVariable()) {
            if (l->name == r->name) {
                continue; }
            substitution[l->name] = r;
            continue;
        }

        // ELIMINATE X = t
        if (l->isVariable()) {
            if (occurs(l->name, r)) { // OCCUR-CHECK не входит ли X в правую часть
                return false; }
            substitution[l->name] = r;
            continue;
        }
        // Теперь обе стороны должны быть функциями
        if (l->isFunction() && r->isFunction()) {
            // конфликт, если функции разные
            if (l->name != r->name) {
                return false;
            }
            // конфликт, если разное количество переменных
            if (l->args.size() != r->args.size()) {
                return false;
            }

            // DECOMPOSE
            for (size_t i = 0; i < l->args.size(); ++i) {
                equations.push_back({l->args[i],r->args[i]});
            }
            continue;
        }
        // Если сюда попали, структуры несовместимы
        return false;
    }
    return true;
}

const Unifier::Substitution& Unifier::getSubstitution() const { //возвращает найденную подстановку
    return substitution;
}

bool Unifier::occurs(const std::string& variable, std::shared_ptr<Term> term) {
    if (term->isVariable()) {
        return term->name == variable;
    }
    for (const auto& argument : term->args) {
        if (occurs(variable, argument)) {
            return true;
        }
    }
    return false;
}

std::shared_ptr<Term> Unifier::substitute(std::shared_ptr<Term> term) {
    if (term->isVariable()) {
        auto it = substitution.find(term->name);
        if (it == substitution.end()) {
            return term;
        }
        return substitute(it->second);
    }
    std::vector<std::shared_ptr<Term>> newArguments;
    for (const auto& argument : term->args) {
        newArguments.push_back(substitute(argument));
    }
    return Term::Fun(term->name,newArguments);
}

bool Unifier::equalTerms(const std::shared_ptr<Term>& left, const std::shared_ptr<Term>& right) {
    // Если один из них variable, а другой function, то деревья разные
    if (left->type != right->type) {
        return false;
    }
    // Если это переменные, то сравниваем их имена
    if (left->isVariable()) {
        return left->name == right->name;
    }
    // Если это функции, то сначала сравниваем имя функции
    if (left->name != right->name) {
        return false;
    }
    // Затем количество детей
    if (left->args.size() != right->args.size()) {
        return false;
    }
    // И рекурсивно сравниваем детей
    for (size_t i = 0; i < left->args.size(); ++i) {
        if (!equalTerms(left->args[i], right->args[i])) {
            return false;
        }
    }
    return true;
}