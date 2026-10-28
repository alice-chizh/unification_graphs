#include "BisimulationMinimizer.h"
#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <tuple>
#include <vector>

std::shared_ptr<Term>
BisimulationMinimizer::minimize(const std::shared_ptr<Term>& root) {
    vertices.clear();
    classId.clear();
    collectVertices(root); // собираем все вершины, достижимые из корневой вершины
    initializeClasses(); // создаём первоначальное разбиение по типу и имени вершины
    while (refineClasses()) { // уточняем классы, пока разбиение не перестанет изменяться
    }
    return buildMinimalGraph(root); // создаём минимальный граф
}

void BisimulationMinimizer::collectVertices(const std::shared_ptr<Term>& root) {
    std::set<Term*> visited;
    std::vector<std::shared_ptr<Term>> stack;
    stack.push_back(root);

    while (!stack.empty()) {
        auto current = stack.back();
        stack.pop_back();
        Term* pointer = current.get();

        if (visited.find(pointer) != visited.end()) { // если эту вершину уже посещали, повторно её не обрабатываем
            continue;
        }
        visited.insert(pointer);
        vertices.push_back(current); // запоминаем вершину
        for (const auto& child : current->args) { // добавляем всех детей в стек
            stack.push_back(child);
        }
    }
}


void BisimulationMinimizer::initializeClasses() {
    std::map<std::tuple<TermType, std::string>, ClassId> classes;
    ClassId nextClass = 0;

    for (const auto& vertex : vertices) {
        auto key = std::make_tuple(vertex->type, vertex->name);
        auto it = classes.find(key);

        if (it == classes.end()) {
            classes[key] = nextClass; // такого класса ещё нет
            classId[vertex.get()] = nextClass;
            ++nextClass;
        } else {
            classId[vertex.get()] = it->second; // такой класс уже есть
        }
    }
}


bool BisimulationMinimizer::refineClasses() {
    using Signature = std::vector<int>;
    std::map<std::tuple<TermType, std::string, Signature>, ClassId> newClasses;

    std::unordered_map<Term*, ClassId> newClassId;
    ClassId nextClass = 0;

    for (const auto& vertex : vertices) {
        Signature signature;
        for (const auto& child : vertex->args) { // добавляем классы всех детей в порядке их расположения
            signature.push_back(classId.at(child.get()));
        }

        auto key = std::make_tuple(vertex->type,vertex->name,signature);

        auto it = newClasses.find(key);

        if (it == newClasses.end()) {
            newClasses[key] = nextClass; // такая сигнатура встречается впервые
            newClassId[vertex.get()] = nextClass;
            ++nextClass;
        } else {
            newClassId[vertex.get()] = it->second; // такая сигнатура уже есть
        }
    }

    // проверяем, изменилось ли само разбиение (остались ли две вершины в одном классе)
    bool changed = false;

    for (const auto& left : vertices) {
        for (const auto& right : vertices) {
            bool oldSame = classId[left.get()] == classId[right.get()];

            bool newSame = newClassId[left.get()] == newClassId[right.get()];

            if (oldSame != newSame) {
                changed = true;
                break;
            }
        }
        if (changed) {
            break;
        }
    }

    classId = std::move(newClassId); // новое разбиение становится текущим
    return changed;
}

std::shared_ptr<Term>
BisimulationMinimizer::buildMinimalGraph(const std::shared_ptr<Term>& root) {
    ClassId maxClass = -1; // находим максимальный номер класса

    for (const auto& vertex : vertices) {
        maxClass = std::max(maxClass, classId.at(vertex.get()));
    }

    std::vector<std::shared_ptr<Term>> newVertices(maxClass + 1);

    // создаём по одной вершине для каждого класса
    for (const auto& vertex : vertices) {
        ClassId id = classId.at(vertex.get());

        if (newVertices[id] == nullptr) { // если вершина для этого класса ещё не создана
            if (vertex->isVariable()) {
                newVertices[id] = Term::Var(vertex->name);
            } else {
                newVertices[id] = Term::Fun(vertex->name);
            }
        }
    }

    std::set<ClassId> built;


    for (const auto& vertex : vertices) { // соединяем новые вершины
        ClassId id = classId.at(vertex.get());
        if (built.find(id) != built.end()) {
            continue;
        }
        built.insert(id);
        auto& newVertex = newVertices[id];

        for (const auto& child : vertex->args) { // для каждого исходящего ребра смотрим, в какой класс попадает его конец
            ClassId childClass = classId.at(child.get());
            newVertex->args.push_back(newVertices[childClass]);
        }
    }

    ClassId rootClass = classId.at(root.get()); // находим класс исходной корневой вершины
    return newVertices[rootClass]; // корнем минимального графа будет вершина этого класса
}