#ifndef BISIMULATION_H
#define BISIMULATION_H
#include "Term.h"
#include <memory>
#include <set>
#include <utility>

class Bisimulation {
public:
    bool check(const std::shared_ptr<Term>& left, const std::shared_ptr<Term>& right); //являются ли бисимиларными

private:
    using Pair = std::pair<const Term*, const Term*>;
    std::set<Pair> visited;
    bool checkRecursive( //соответствуют ли друг другу эти две вершины
            const std::shared_ptr<Term>& left,
            const std::shared_ptr<Term>& right
    );
};

#endif