#ifndef BISIMULATION_MINIMIZER_H
#define BISIMULATION_MINIMIZER_H
#include "Term.h"
#include <memory>
#include <unordered_map>
#include <vector>

class BisimulationMinimizer {
public:
    std::shared_ptr<Term> minimize(const std::shared_ptr<Term>& root);

private:
    using ClassId = int;

    std::vector<std::shared_ptr<Term>> vertices; // все вершины исходного графа
    std::unordered_map<Term*, ClassId> classId; // какому классу принадлежит каждая вершина

    void collectVertices(const std::shared_ptr<Term>& root); // все вершины, достижимые из root
    void initializeClasses(); // первоначальное разбиение по type + name

    bool refineClasses(); // возвращает true, если классы изменились

    std::shared_ptr<Term> buildMinimalGraph(const std::shared_ptr<Term>& root); // строит минимальный граф по найденным классам
};

#endif
