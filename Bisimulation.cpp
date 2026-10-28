#include "Bisimulation.h"

bool Bisimulation::check(const std::shared_ptr<Term>& left, const std::shared_ptr<Term>& right) {
    visited.clear();
    return checkRecursive(left, right);
}

bool Bisimulation::checkRecursive(const std::shared_ptr<Term>& left, const std::shared_ptr<Term>& right) {
    Pair pair{left.get(), right.get()};
    if (visited.find(pair) != visited.end()) { // если эту пару вершин уже проверяли, не нужно заходить в цикл снова.
        return true;
    }
    visited.insert(pair); // запоминаем пару вершин.
    if (left->type != right->type) { // типы вершин должны совпадать
        return false;
    }
    if (left->name != right->name) { // метки вершин должны совпадать
        return false;
    }
    if (left->args.size() != right->args.size()) { // количество исходящих рёбер должно совпадать
        return false;
    }
    for (size_t i = 0; i < left->args.size(); ++i) { // проверяем соответствующие рёбра
        if (!checkRecursive(left->args[i],right->args[i])) {
            return false;
        }
    }
    return true;
}