#include <fstream>
#include <iostream>
#include <string>
#include "Bisimulation.h"
#include "GraphParser.h"
#include "GraphUnifier.h"

int main(int argc, char** argv) {
    std::string fileName = argc > 1 ? argv[1] : "tests.txt";
    std::ifstream input(fileName);
    if (!input.is_open()) {
        std::cerr << "Cannot open " << fileName << "\n";
        return 1;
    }
    std::string line;
    int testNumber = 1;
    while (std::getline(input, line)) {
        size_t first = line.find_first_not_of(" \t\r");
        if (first == std::string::npos || line[first] == '#') {
            continue;
        }
        std::cout << "Test " << testNumber++ << ": " << line << "\n";
        try {
            GraphParser parser(line);
            auto system = parser.parseSystem();
            GraphUnifier unifier;
            if (!unifier.unify(system)) {
                std::cout << "  Result: FAILURE (" << unifier.getFailureReason() << ")\n\n";
                continue;
            }

            std::cout << "  Result: SUCCESS\n";
            auto substitution = unifier.getSubstitution();
            if (!substitution.empty()) {
                std::cout << "  Substitution:\n";
                for (const auto& [variable, value] : substitution) {
                    std::cout << "    " << variable << " -> " << value->toString() << "\n";
                }
            }

            auto unified = unifier.getUnifiedTerm();
            std::cout << "  Unified graph: " << unified->toString() << "\n";
            std::cout << "  Unfolding:     " << unified->unfold(4) << "\n";

            // проверка: развёртки σ(l) и σ(r) совпадают <=> графы бисимилярны.
            bool verified = true;
            for (const auto& equation : system) {
                Bisimulation bisimulation;
                verified = verified && bisimulation.check(
                        unifier.apply(equation.left),
                        unifier.apply(equation.right));
            }

            std::cout << "  Check sigma(l) ~ sigma(r): " << (verified ? "OK" : "WRONG") << "\n\n";

        } catch (const std::exception& error) {
            std::cout << "  ERROR: " << error.what() << "\n\n";
        }
    }
    return 0;
}