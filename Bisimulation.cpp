#include "Bisimulation.h"

bool Bisimulation::check(
        const std::shared_ptr<Term>& left,
        const std::shared_ptr<Term>& right
) {
    visited.clear();

    return checkRecursive(left, right);
}

bool Bisimulation::checkRecursive(
        const std::shared_ptr<Term>& left,
        const std::shared_ptr<Term>& right
) {
    Pair pair{left.get(), right.get()};

    // Если эту пару вершин уже проверяли,
    // не нужно заходить в цикл снова.
    if (visited.find(pair) != visited.end()) {
        return true;
    }

    // Запоминаем пару вершин.
    visited.insert(pair);

    // Типы вершин должны совпадать:
    // VARIABLE с VARIABLE
    // FUNCTION с FUNCTION
    if (left->type != right->type) {
        return false;
    }

    // Метки вершин должны совпадать.
    if (left->name != right->name) {
        return false;
    }

    // Количество исходящих рёбер должно совпадать.
    if (left->args.size() != right->args.size()) {
        return false;
    }

    // Проверяем соответствующие рёбра.
    for (size_t i = 0; i < left->args.size(); ++i) {
        if (!checkRecursive(
                left->args[i],
                right->args[i]
        )) {
            return false;
        }
    }

    return true;
}