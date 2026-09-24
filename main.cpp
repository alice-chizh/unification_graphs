/*#include "Term.h"
#include "Unifier.h"
#include <iostream>
#include <string>
void test(const std::string& name, std::shared_ptr<Term> left, std::shared_ptr<Term> right) {
        std::cout << "==============================\n";
        std::cout << "TEST: " << name << "\n";
        std::cout << "Equation:\n";
        std::cout << left->toString() << " = " << right->toString() << "\n";
        Unifier unifier;

        bool success = unifier.unify(left, right);
        if (!success) {
            std::cout << "Result: NO UNIFIER\n";
        } else {
            std::cout << "Result: SUCCESS\n";
            std::cout << "Substitution:\n";
            for (const auto& [variable, term] : unifier.getSubstitution()) {
                std::cout << "  " << variable << " -> " << term->toString() << "\n";
            }
        }
        std::cout << "\n";
    }
    int main() {
       /* auto X = Term::Var("X");
        auto fX = Term::Fun("f", {X});
        test("OCCURS-CHECK",X,fX);*/


        /*auto X = Term::Var("X");
        auto Y = Term::Var("Y");

        auto a = Term::Fun("a");
        auto b = Term::Fun("b");

        auto gY = Term::Fun("g", {Y});
        auto gb = Term::Fun("g", {b});

        auto left = Term::Fun("f", {X, gY});
        auto right = Term::Fun("f", {a, gb});

        test("FULL EXAMPLE", left, right);

        auto X = Term::Var("X");
        auto a = Term::Fun("a");
        auto b = Term::Fun("b");

        auto left = Term::Fun("f", {X, X});
        auto right = Term::Fun("f", {a, b});

        test("REPEATED VARIABLE CLASH", left, right);
    }
    */

#include <fstream>
#include <iostream>
#include <string>

#include "Parser.h"
#include "Unifier.h"

int main() {
    std::ifstream input("tests.txt");
    if (!input.is_open()) {
        std::cerr << "Cannot open tests.txt\n";
        return 1;
    }

    std::string line;
    int testNumber = 1;

    while (std::getline(input, line)) {
        // Пропускаем пустые строки
        if (line.empty()) {
            continue;
        }

        try {
            Parser parser(line);

            Equation equation = parser.parseEquation();

            Unifier unifier;

            bool result = unifier.unify(
                    equation.left,
                    equation.right
            );

            std::cout
                    << "Test "
                    << testNumber
                    << ": "
                    << equation.left->toString()
                    << " = "
                    << equation.right->toString()
                    << "\n";

            if (result) {
                std::cout << "  Result: SUCCESS\n";

                const auto& substitution =
                        unifier.getSubstitution();

                if (!substitution.empty()) {
                    std::cout << "  Substitution:\n";

                    for (const auto& [variable, term]
                            : substitution) {

                        std::cout
                                << "    "
                                << variable
                                << " -> "
                                << term->toString()
                                << "\n";
                    }
                }
            } else {
                std::cout << "  Result: FAILURE\n";
            }

            std::cout << "\n";

        } catch (const std::exception& error) {
            std::cerr
                    << "Test "
                    << testNumber
                    << ": ERROR\n";

            std::cerr
                    << "  "
                    << error.what()
                    << "\n\n";
        }

        ++testNumber;
    }

    return 0;
}