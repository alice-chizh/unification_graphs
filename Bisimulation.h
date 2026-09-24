#ifndef BISIMULATION_H
#define BISIMULATION_H

#include "Term.h"

#include <memory>
#include <set>
#include <utility>

class Bisimulation {
public:
    bool check(
            const std::shared_ptr<Term>& left,
            const std::shared_ptr<Term>& right
    );

private:
    using Pair = std::pair<const Term*, const Term*>;

    std::set<Pair> visited;

    bool checkRecursive(
            const std::shared_ptr<Term>& left,
            const std::shared_ptr<Term>& right
    );
};

#endif