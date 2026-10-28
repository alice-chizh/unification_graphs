#ifndef TERM_H
#define TERM_H
#include <string>
#include <vector>
#include <memory>

enum class TermType {
    VARIABLE,
    FUNCTION
};

struct Term {
    TermType type;
    std::string name;
    std::vector<std::shared_ptr<Term>> args;
    static std::shared_ptr<Term> Var(const std::string& name);
    static std::shared_ptr<Term> Fun(const std::string& name, const std::vector<std::shared_ptr<Term>>& args = {});
    bool isVariable() const;
    bool isFunction() const;
    std::string toString() const;
    std::string unfold(int depth) const; // первые depth уровней универсального накрывающего дерева (развёртки).
    void validate() const; // переменные допускаются только в вершинах без исходящих дуг
    static std::vector<std::shared_ptr<Term>> collectNodes(const std::shared_ptr<Term>& root); // все вершины, достижимые из root, каждая ровно один раз
};

struct Equation { // уравнение left = right
    std::shared_ptr<Term> left;
    std::shared_ptr<Term> right;
};

#endif




/*struct Term {
    enum Type {
        VARIABLE,
        FUNCTION
    };
    Type type;
    std::string name;
    std::vector<std::shared_ptr<Term>> args;
};
static shared_ptr<Term> Var(const string& name);
static shared_ptr<Term> Fun (const string& name, vector<shared_ptr<Term>> args = { });*/