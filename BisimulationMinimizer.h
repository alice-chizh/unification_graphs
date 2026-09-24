#ifndef BISIMULATION_MINIMIZER_H
#define BISIMULATION_MINIMIZER_H

#include "Term.h"

#include <memory>
#include <unordered_map>
#include <vector>

class BisimulationMinimizer {
public:
    std::shared_ptr<Term> minimize(
            const std::shared_ptr<Term>& root
    );

private:
    using ClassId = int;

    // Все вершины исходного графа.
    std::vector<std::shared_ptr<Term>> vertices;

    // Какому классу принадлежит каждая вершина.
    std::unordered_map<Term*, ClassId> classId;

    // Собираем все вершины, достижимые из root.
    void collectVertices(
            const std::shared_ptr<Term>& root
    );

    // Первоначальное разбиение по type + name.
    void initializeClasses();

    // Одно уточнение разбиения.
    // Возвращает true, если классы изменились.
    bool refineClasses();

    // Строит минимальный граф по найденным классам.
    std::shared_ptr<Term> buildMinimalGraph(
            const std::shared_ptr<Term>& root
    );
};

#endif
