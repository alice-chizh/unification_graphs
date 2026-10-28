#ifndef GRAPH_UNIFIER_H
#define GRAPH_UNIFIER_H
#include "Term.h"
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class GraphUnifier {
public:
    bool unify(const std::shared_ptr<Term>& left, const std::shared_ptr<Term>& right);
    bool unify(const std::vector<Equation>& system);

    std::map<std::string, std::shared_ptr<Term>> getSubstitution() const;
    std::shared_ptr<Term> getUnifiedTerm(size_t equationIndex = 0) const;
    std::shared_ptr<Term> apply(const std::shared_ptr<Term>& term) const;
    const std::string& getFailureReason() const;

private:
    std::vector<std::shared_ptr<Term>> nodes;
    std::unordered_map<const Term*, int> indexOf;

    std::vector<int> parent;
    std::vector<int> size;

    std::vector<int> schema;
    std::map<std::string, int> variableNode;
    std::vector<Equation> equations;
    std::unordered_map<int, std::shared_ptr<Term>> classTerm;
    std::string failureReason;

    void clear();
    int addNode(const std::shared_ptr<Term>& term);
    int find(int vertex);
    int findConst(int vertex) const;
    bool merge(int left, int right, std::vector<std::pair<int, int>>& pending);
    void buildQuotient();
};

#endif