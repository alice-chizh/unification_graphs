#include "BisimulationMinimizer.h"

#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <tuple>
#include <vector>

std::shared_ptr<Term>
BisimulationMinimizer::minimize(
        const std::shared_ptr<Term>& root
) {
    vertices.clear();
    classId.clear();

    // 1. Собираем все вершины,
    // достижимые из корневой вершины.
    collectVertices(root);

    // 2. Создаём первоначальное разбиение
    // по типу и имени вершины.
    initializeClasses();

    // 3. Уточняем классы, пока разбиение
    // не перестанет изменяться.
    while (refineClasses()) {
    }

    // 4. Создаём минимальный граф.
    return buildMinimalGraph(root);
}


void BisimulationMinimizer::collectVertices(
        const std::shared_ptr<Term>& root
) {
    std::set<Term*> visited;

    std::vector<std::shared_ptr<Term>> stack;
    stack.push_back(root);

    while (!stack.empty()) {
        auto current = stack.back();
        stack.pop_back();

        Term* pointer = current.get();

        // Если эту вершину уже посещали,
        // повторно её не обрабатываем.
        if (visited.find(pointer) != visited.end()) {
            continue;
        }

        visited.insert(pointer);

        // Запоминаем вершину.
        vertices.push_back(current);

        // Добавляем всех детей в стек.
        for (const auto& child : current->args) {
            stack.push_back(child);
        }
    }
}


void BisimulationMinimizer::initializeClasses() {
    // Ключ:
    // (тип вершины, имя вершины)
    //
    // Например:
    //
    // (FUNCTION, "a") -> класс 0
    // (FUNCTION, "b") -> класс 1
    // (FUNCTION, "f") -> класс 2
    // (VARIABLE, "X") -> класс 3

    std::map<
            std::tuple<TermType, std::string>,
            ClassId
    > classes;

    ClassId nextClass = 0;

    for (const auto& vertex : vertices) {
        auto key = std::make_tuple(
                vertex->type,
                vertex->name
        );

        auto it = classes.find(key);

        if (it == classes.end()) {
            // Такого класса ещё нет.
            classes[key] = nextClass;

            classId[vertex.get()] = nextClass;

            ++nextClass;
        } else {
            // Такой класс уже существует.
            classId[vertex.get()] = it->second;
        }
    }
}


bool BisimulationMinimizer::refineClasses() {
    // Сигнатура вершины:
    //
    // (type,
    //     name,
    //     class(child0),
    //     class(child1),
    //     ...)
    //
    // Например:
    //
    // f(a, b)
    //
    // если:
    //
    // a -> класс 0
    // b -> класс 1
    //
    // имеет сигнатуру:
    //
    // (FUNCTION, "f", 0, 1)

    using Signature = std::vector<int>;

    std::map<
            std::tuple<TermType, std::string, Signature>,
            ClassId
    > newClasses;

    std::unordered_map<Term*, ClassId> newClassId;

    ClassId nextClass = 0;

    for (const auto& vertex : vertices) {
        Signature signature;

        // Добавляем классы всех детей
        // в порядке их расположения.
        for (const auto& child : vertex->args) {
            signature.push_back(
                    classId.at(child.get())
            );
        }

        auto key = std::make_tuple(
                vertex->type,
                vertex->name,
                signature
        );

        auto it = newClasses.find(key);

        if (it == newClasses.end()) {
            // Такая сигнатура встречается впервые.
            newClasses[key] = nextClass;

            newClassId[vertex.get()] = nextClass;

            ++nextClass;
        } else {
            // Такая сигнатура уже существует.
            newClassId[vertex.get()] = it->second;
        }
    }

    // Проверяем, изменилось ли само разбиение.
    //
    // Нас интересует не номер класса,
    // а то, остались ли две вершины
    // в одном классе.
    bool changed = false;

    for (const auto& left : vertices) {
        for (const auto& right : vertices) {
            bool oldSame =
                    classId[left.get()] ==
                    classId[right.get()];

            bool newSame =
                    newClassId[left.get()] ==
                    newClassId[right.get()];

            if (oldSame != newSame) {
                changed = true;
                break;
            }
        }

        if (changed) {
            break;
        }
    }

    // Новое разбиение становится текущим.
    classId = std::move(newClassId);

    return changed;
}


std::shared_ptr<Term>
BisimulationMinimizer::buildMinimalGraph(
        const std::shared_ptr<Term>& root
) {
    // Находим максимальный номер класса.
    ClassId maxClass = -1;

    for (const auto& vertex : vertices) {
        maxClass = std::max(
                maxClass,
                classId.at(vertex.get())
        );
    }

    // Для каждого класса создаём
    // одну новую вершину.
    //
    // Например:
    //
    // класс 0 -> newVertices[0]
    // класс 1 -> newVertices[1]
    // класс 2 -> newVertices[2]
    std::vector<std::shared_ptr<Term>> newVertices(
            maxClass + 1
    );

    // Создаём по одной вершине
    // для каждого класса.
    for (const auto& vertex : vertices) {
        ClassId id = classId.at(vertex.get());

        // Если вершина для этого класса
        // ещё не создана.
        if (newVertices[id] == nullptr) {
            if (vertex->isVariable()) {
                newVertices[id] =
                        Term::Var(vertex->name);
            } else {
                newVertices[id] =
                        Term::Fun(vertex->name);
            }
        }
    }

    // Здесь запоминаем классы,
    // которым мы уже построили исходящие рёбра.
    std::set<ClassId> built;

    // Теперь соединяем новые вершины.
    for (const auto& vertex : vertices) {
        ClassId id = classId.at(vertex.get());

        // Если этот класс уже обработан,
        // второй раз его строить не надо.
        if (built.find(id) != built.end()) {
            continue;
        }

        built.insert(id);

        auto& newVertex = newVertices[id];

        // Для каждого исходящего ребра
        // смотрим, в какой класс попадает его конец.
        for (const auto& child : vertex->args) {
            ClassId childClass =
                    classId.at(child.get());

            // Ребро новой вершины идёт
            // на вершину соответствующего класса.
            newVertex->args.push_back(
                    newVertices[childClass]
            );
        }
    }

    // Находим класс исходной корневой вершины.
    ClassId rootClass =
            classId.at(root.get());

    // Корнем минимального графа будет
    // вершина этого класса.
    return newVertices[rootClass];
}