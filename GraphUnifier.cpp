#include "GraphUnifier.h"
#include "BisimulationMinimizer.h"
#include "Term.h"
#include <utility>

bool GraphUnifier::unify(const std::shared_ptr<Term>& left, const std::shared_ptr<Term>& right) {
    return unify(std::vector<Equation>{{left, right}});
}

void GraphUnifier::clear() {
    nodes.clear();
    indexOf.clear();
    parent.clear();
    size.clear();
    schema.clear();
    variableNode.clear();
    equations.clear();
    classTerm.clear();
    failureReason.clear();
}

int GraphUnifier::addNode(const std::shared_ptr<Term>& term) {
    auto it = indexOf.find(term.get());

    if (it != indexOf.end()) {
        return it->second;
    }

    int index = static_cast<int>(nodes.size());

    nodes.push_back(term);
    indexOf[term.get()] = index;
    parent.push_back(index);
    size.push_back(1);
    schema.push_back(term->isFunction() ? index : -1);

    return index;
}

int GraphUnifier::find(int vertex) {
    while (parent[vertex] != vertex) {
        parent[vertex] = parent[parent[vertex]]; // сжатие путей
        vertex = parent[vertex];
    }
    return vertex;
}

int GraphUnifier::findConst(int vertex) const {
    while (parent[vertex] != vertex) {
        vertex = parent[vertex];
    }
    return vertex;
}

// слияние двух мультиуравнений.
bool GraphUnifier::merge(int left, int right, std::vector<std::pair<int, int>>& pending) {
    int a = find(left);
    int b = find(right);

    // уже в одном классе: уравнение выполнено (или будет выполнено). проверка обеспечивает остановку на циклах.
    if (a == b) {
        return true;
    }
    int schemaA = schema[a];
    int schemaB = schema[b];

    // CLASH: разные функциональные символы или арности.
    if (schemaA != -1 && schemaB != -1) {
        const auto& s = nodes[schemaA];
        const auto& t = nodes[schemaB];
        if (s->name != t->name || s->args.size() != t->args.size()) {
            failureReason = "clash: " + s->name + "/" + std::to_string(s->args.size()) + " vs " + t->name + "/" + std::to_string(t->args.size());
            return false;
        }
    }

    // Объединение по размеру.
    if (size[a] < size[b]) {
        std::swap(a, b);
    }
    parent[b] = a;
    size[a] += size[b];
    schema[a] = schemaA != -1 ? schemaA : schemaB;

    if (schemaA != -1 && schemaB != -1) {
        const auto& s = nodes[schemaA];
        const auto& t = nodes[schemaB];
        for (size_t i = 0; i < s->args.size(); ++i) {
            pending.emplace_back(indexOf.at(s->args[i].get()),indexOf.at(t->args[i].get()));
        }
    }
    return true;
}

bool GraphUnifier::unify(const std::vector<Equation>& system) {
    clear();
    equations = system;
    // Собираем вершины всех графов.
    for (const auto& equation : equations) {
        for (const auto& side : {equation.left, equation.right}) {
            side->validate();
            for (const auto& node : Term::collectNodes(side)) {
                addNode(node);
            }
        }
    }
    std::vector<std::pair<int, int>> pending;
    // Вершины-переменные с одинаковым именем — это одна переменная.
    for (int i = 0; i < static_cast<int>(nodes.size()); ++i) {
        if (!nodes[i]->isVariable()) {
            continue;
        }
        auto [it, inserted] = variableNode.emplace(nodes[i]->name, i);
        if (!inserted) {
            pending.emplace_back(it->second, i);
        }
    }

    // Исходные уравнения.
    for (const auto& equation : equations) {
        pending.emplace_back(indexOf.at(equation.left.get()),
                             indexOf.at(equation.right.get()));
    }
    // Основной цикл. Каждое успешное слияние уменьшает число классов
    while (!pending.empty()) {
        auto [left, right] = pending.back();
        pending.pop_back();
        if (!merge(left, right, pending)) {
            return false;
        }
    }
    buildQuotient();
    return true;
}

void GraphUnifier::buildQuotient() {

    std::unordered_map<int, std::string> classVariable;

    for (const auto& [name, vertex] : variableNode) {
        classVariable.emplace(find(vertex), name); // map упорядочен => первое — минимальное
    }

    for (int i = 0; i < static_cast<int>(nodes.size()); ++i) {
        int root = find(i);
        if (classTerm.count(root)) {
            continue;
        }
        if (schema[root] == -1) {
            classTerm[root] = Term::Var(classVariable.at(root));
        } else {
            classTerm[root] = Term::Fun(nodes[schema[root]]->name);
        }
    }

    for (auto& [root, term] : classTerm) {
        if (schema[root] == -1) {
            continue;
        }
        for (const auto& child : nodes[schema[root]]->args) {
            term->args.push_back(classTerm.at(find(indexOf.at(child.get()))));
        }
    }
}

std::map<std::string, std::shared_ptr<Term>> GraphUnifier::getSubstitution() const {
    std::map<std::string, std::shared_ptr<Term>> result;

    for (const auto& [name, vertex] : variableNode) {
        const auto& value = classTerm.at(findConst(vertex));

        if (value->isVariable() && value->name == name) {
            continue;
        }
        BisimulationMinimizer minimizer;
        result[name] = minimizer.minimize(value);
    }
    return result;
}

std::shared_ptr<Term> GraphUnifier::getUnifiedTerm(size_t equationIndex) const {
    const auto& left = equations.at(equationIndex).left;
    BisimulationMinimizer minimizer;
    return minimizer.minimize(classTerm.at(findConst(indexOf.at(left.get()))));
}

std::shared_ptr<Term> GraphUnifier::apply(const std::shared_ptr<Term>& term) const {
    std::unordered_map<const Term*, std::shared_ptr<Term>> memo;

    for (const auto& node : Term::collectNodes(term)) {
        if (node->isVariable()) {
            auto it = variableNode.find(node->name);
            memo[node.get()] = it == variableNode.end()
                               ? node
                               : classTerm.at(findConst(it->second));
        } else {
            memo[node.get()] = Term::Fun(node->name);
        }
    }

    for (const auto& node : Term::collectNodes(term)) {
        if (node->isFunction()) {
            for (const auto& child : node->args) {
                memo[node.get()]->args.push_back(memo.at(child.get()));
            }
        }
    }
    return memo.at(term.get());
}

const std::string& GraphUnifier::getFailureReason() const {
    return failureReason;
}