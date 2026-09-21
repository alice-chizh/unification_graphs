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