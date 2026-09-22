#ifndef UNIFIER_H
#define UNIFIER_H
#include "Term.h"
#include <deque>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

struct Equation {
    std::shared_ptr<Term> left;
    std::shared_ptr<Term> right;
};

class Unifier {
public:
    using Substitution = std::unordered_map<std::string, std::shared_ptr<Term>>;

    bool unify( //унифицировать два терма
            std::shared_ptr<Term> left,
            std::shared_ptr<Term> right
    );
    const Substitution& getSubstitution() const; //получить найденную подстановку

private:
    std::deque<Equation> equations; //очередь текущих уравнений
    Substitution substitution; //замена
    bool equalTerms(const std::shared_ptr<Term>& left, const std::shared_ptr<Term>& right); //сравнение термов
    bool occurs(const std::string& variable, std::shared_ptr<Term> term); //встречается ли переменная внутри терма
    std::shared_ptr<Term> substitute(std::shared_ptr<Term> term); //применяем найденную подстановку к терму
};

#endif