#include "Term.h"
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

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
namespace {

// Если при обходе мы приходим в вершину, которая сейчас лежит на стеке
// рекурсии, значит найдена обратная дуга (цикл): печатаем ссылку $k,
// а при выходе из этой вершины добавляем к ней метку "$k:".
    class CyclePrinter {
    public:
        std::string print(const Term* term) {
            if (onStack.count(term)) {
                referenced.insert(term);
                return "$" + std::to_string(idOf(term));
            }

            if (term->args.empty()) {
                return term->name;
            }

            onStack.insert(term);

            std::string result = term->name + "(";

            for (size_t i = 0; i < term->args.size(); ++i) {
                if (i > 0) {
                    result += ", ";
                }
                result += print(term->args[i].get());
            }

            result += ")";

            onStack.erase(term);

            if (referenced.erase(term)) {
                result = "$" + std::to_string(idOf(term)) + ":" + result;
            }

            return result;
        }

    private:
        std::unordered_map<const Term*, int> ids;
        std::unordered_set<const Term*> onStack;
        std::unordered_set<const Term*> referenced;

        int idOf(const Term* term) {
            auto [it, inserted] = ids.emplace(term, static_cast<int>(ids.size()) + 1);
            return it->second;
        }
    };

}

std::string Term::toString() const {
    CyclePrinter printer;
    return printer.print(this);
}

std::string Term::unfold(int depth) const {
    if (args.empty()) {
        return name; }
    if (depth == 0) {
        return "..."; }
    std::string result = name + "(";
    for (size_t i = 0; i < args.size(); ++i) {
        if (i > 0) {
            result += ", "; }
        result += args[i]->unfold(depth - 1);
    }
    return result + ")";
}

std::vector<std::shared_ptr<Term>> Term::collectNodes(const std::shared_ptr<Term>& root) {
    std::vector<std::shared_ptr<Term>> result;
    std::unordered_set<const Term*> visited;
    std::vector<std::shared_ptr<Term>> stack{root};
    while (!stack.empty()) {
        auto current = stack.back();
        stack.pop_back();
        if (!visited.insert(current.get()).second) {
            continue; }
        result.push_back(current);
        for (const auto& child : current->args) {
            stack.push_back(child);
        }
    }
    return result;
}

void Term::validate() const {
    std::unordered_set<const Term*> visited;
    std::vector<const Term*> stack{this};
    while (!stack.empty()) {
        const Term* node = stack.back();
        stack.pop_back();
        if (!visited.insert(node).second) {
            continue; }
        if (node->isVariable() && !node->args.empty()) {
            throw std::runtime_error("Variable " + node->name + " has outgoing edges");
        }
        if (node->isFunction() && node->name.empty()) {
            throw std::runtime_error("Function node without a label");
        }
        for (const auto& child : node->args) {
            stack.push_back(child.get());
        }
    }
}